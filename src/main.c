//
// Created by Kıvanç TÜRKER on 20.05.2024.
//
#include <errno.h>
#include <pthread.h>
#include <queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include "myutil.h"
#include "fileops.h"
#include "worker.h"
#include "manager.h"

int isTerminates = 0;
pthread_cond_t terminationCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t terminationMutex = PTHREAD_MUTEX_INITIALIZER;

void sigintHandler(int signal) {
    pthread_mutex_lock(&terminationMutex);
    isTerminates = 1;
    pthread_cond_signal(&terminationCond);
    pthread_mutex_unlock(&terminationMutex);
}

// Main with arguments
int main(int argc, char *argv[]) {
    clock_t start, end;
    double cpuTimeUsed;
    if (argc < 5) {
        printf("Usage: %s <bufferSize> <threadCount> <srcPath> <destPath>\n", argv[0]);
        return 1;
    }

    start = clock();

    // Signal handling for SIGINT
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &sigintHandler;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    struct Args args;
    parseArgs(argc, argv, &args);

    prepareDirectory(args.destPath);
    struct Queue bufferQueue;
    initQueue(&bufferQueue, args.bufferSize);

    int fileInfoSize = 0;
    struct FileInfo* fileInfos = openAllFiles(args.srcPath, args.destPath, &fileInfoSize);
    if (fileInfos == NULL) {
        return 1;
    }

    struct FileStats fileStats;
    fileStats.regularFileCount = 0;
    fileStats.fifoCount = 0;
    fileStats.directoryCount = 0;
    fileStats.totalBytes = 0;
    traverseDirectoryAndFillStats(args.srcPath, &fileStats);

    // Thread pool creation
    pthread_mutex_t bufferMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t byteCounterMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t bufferCond = PTHREAD_COND_INITIALIZER;

    int isFinished = 0;
    off_t byteCounter = 0;

    // Create manager thread
    pthread_t managerThread;
    struct ThreadArgs managerArgs;
    managerArgs.bufferQueue = &bufferQueue;
    managerArgs.bufferMutex = &bufferMutex;
    managerArgs.byteCounterMutex = &byteCounterMutex;
    managerArgs.bufferCond = &bufferCond;
    managerArgs.isFinished = &isFinished;
    managerArgs.byteCounter = &byteCounter;
    managerArgs.terminationMutex = &terminationMutex;
    managerArgs.terminationCond = &terminationCond;
    strncpy(managerArgs.destPath, args.destPath, MAX_DIR_PATH_SIZE);
    strncpy(managerArgs.srcPath, args.srcPath, MAX_DIR_PATH_SIZE);
    managerArgs.fileInfos = fileInfos;
    managerArgs.fileInfoSize = fileInfoSize;

    int threadCreationResult = pthread_create(&managerThread, NULL, manager, &managerArgs);
    if (threadCreationResult != 0) {
        fprintf(stderr, "Error: pthread_create failed with error number %d\n", threadCreationResult);
        return 1;
    }


    // Create worker threads
    pthread_t* workerThreads = malloc(args.threadCount * sizeof(pthread_t));
    struct ThreadArgs* threadArgsArray = malloc(args.threadCount * sizeof(struct ThreadArgs));

    for (int i = 0; i < args.threadCount; i++) {
        threadArgsArray[i].bufferQueue = &bufferQueue;
        threadArgsArray[i].bufferMutex = &bufferMutex;
        threadArgsArray[i].byteCounterMutex = &byteCounterMutex;
        threadArgsArray[i].bufferCond = &bufferCond;
        threadArgsArray[i].isFinished = &isFinished;
        threadArgsArray[i].byteCounter = &byteCounter;
        strncpy(threadArgsArray[i].destPath, args.destPath, MAX_DIR_PATH_SIZE);
        strncpy(threadArgsArray[i].srcPath, args.srcPath, MAX_DIR_PATH_SIZE);

        threadCreationResult = pthread_create(&workerThreads[i], NULL, worker, &threadArgsArray[i]);
        if (threadCreationResult != 0) {
            fprintf(stderr, "Error: pthread_create failed with error number %d\n", threadCreationResult);
            isTerminates = 1;
            break;
        }
    }

    // Wait for the termination signal
    pthread_mutex_lock(&terminationMutex);
    pthread_cond_wait(&terminationCond, &terminationMutex);

    int threadJointResult = -1;
    int threadCancelResult = -1;
    if (isTerminates) {
        threadCancelResult = pthread_cancel(managerThread);
        if (threadCancelResult != 0) {
            fprintf(stderr, "Error: pthread_cancel failed with error number %d\n", threadCancelResult);
        }
        cancelAllThreads(workerThreads, args.threadCount);
        pthread_join(managerThread, NULL);
        joinAllThreads(workerThreads, args.threadCount);
        printf("Program Interrupted. Exiting...\n");

    }
    else {
        // Join all threads
        if (pthread_join(managerThread, NULL) != 0) {
            fprintf(stderr, "Error: pthread_join failed with error number %d\n", threadJointResult);
            cleanUpFileInfo(fileInfos, fileInfoSize);
            free(workerThreads);
            free(threadArgsArray);
            return 1;
        }
        threadJointResult = joinAllThreads(workerThreads, args.threadCount);
        if (threadJointResult != 0) {
            cleanUpFileInfo(fileInfos, fileInfoSize);
            free(workerThreads);
            free(threadArgsArray);
            return 1;
        }
    }

    end = clock();
    cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
    long totalMilliseconds = (long)(cpuTimeUsed * 1000);
    long minutes = totalMilliseconds / 60000;
    totalMilliseconds %= 60000;
    long seconds = totalMilliseconds / 1000;
    long milliseconds = totalMilliseconds % 1000;

    printf("\n---------------STATISTICS--------------------\n");
    printf("Consumers: %d - Buffer Size: %d\n", args.threadCount, args.bufferSize);
    printf("Number of Regular File: %d\n", fileStats.regularFileCount);
    printf("Number of FIFO File: %d\n", fileStats.fifoCount);
    printf("Number of Directory: %d\n", fileStats.directoryCount);
    printf("TOTAL BYTES WRITTEN: %ld\n", (long) byteCounter);
    printf("TOTAL TIME: %02ld:%02ld.%03ld (min:sec.mili)\n", minutes, seconds, milliseconds);

    // Cleanups
    cleanUpFileInfo(fileInfos, fileInfoSize);
    free(workerThreads);
    free(threadArgsArray);

    return 0;
}