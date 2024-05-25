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

int isTerminates = 0;

void sigintHandler(int signal) {
    isTerminates = 1;
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
    traverseDirectoryAndFillStats(args.srcPath, &fileStats);

    // Thread pool creation
    pthread_mutex_t bufferMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t byteCounterMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t bufferCond = PTHREAD_COND_INITIALIZER;

    pthread_t* threads = malloc(args.threadCount * sizeof(pthread_t));
    struct ThreadArgs* threadArgsArray = malloc(args.threadCount * sizeof(struct ThreadArgs));
    int isFinished = 0;
    off_t byteCounter = 0;

    for (int i = 0; i < args.threadCount; i++) {
        threadArgsArray[i].bufferQueue = &bufferQueue;
        threadArgsArray[i].bufferMutex = &bufferMutex;
        threadArgsArray[i].byteCounterMutex = &byteCounterMutex;
        threadArgsArray[i].bufferCond = &bufferCond;
        threadArgsArray[i].isFinished = &isFinished;
        threadArgsArray[i].byteCounter = &byteCounter;
        strncpy(threadArgsArray[i].destPath, args.destPath, MAX_DIR_PATH_SIZE);
        strncpy(threadArgsArray[i].srcPath, args.srcPath, MAX_DIR_PATH_SIZE);

        int threadCreationResult = pthread_create(&threads[i], NULL, worker, &threadArgsArray[i]);
        if (threadCreationResult != 0) {
            fprintf(stderr, "Error: pthread_create failed with error number %d\n", threadCreationResult);
            isTerminates = 1;
            break;
        }
    }

    // Send every file info to buffer queue
    for (int i = 0; i < fileInfoSize && !isTerminates; i++) {
        pthread_mutex_lock(&bufferMutex);
        if (isQueueFull(&bufferQueue)) {
            pthread_cond_wait(&bufferCond, &bufferMutex);
        }
        if (fileInfos[i].type != DIRECTORY) {
            enqueue(&bufferQueue, fileInfos[i]);
            pthread_cond_broadcast(&bufferCond);
        }
        pthread_mutex_unlock(&bufferMutex);
    }
    int threadJointResult = -1;
    if (isTerminates) {
        cancelAllThreads(threads, args.threadCount);
        joinAllThreads(threads, args.threadCount);
        cleanUpFileInfo(fileInfos, fileInfoSize);
        free(threads);
        free(threadArgsArray);
        printf("Program Interrupted. Exiting...\n");

        return 0;
    }

    // Wait until the queue is fully processed
    pthread_mutex_lock(&bufferMutex);
    while (!isQueueEmpty(&bufferQueue)) {
        pthread_cond_wait(&bufferCond, &bufferMutex);
    }
    isFinished = 1;
    pthread_cond_broadcast(&bufferCond);
    pthread_mutex_unlock(&bufferMutex);

    // Join all threads
    threadJointResult = joinAllThreads(threads, args.threadCount);
    if (threadJointResult != 0) {
        cleanUpFileInfo(fileInfos, fileInfoSize);
        free(threads);
        free(threadArgsArray);
        return 1;
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
    printf("TOTAL BYTES ESTIMATED: %lld\n", fileStats.totalBytes);
    printf("TOTAL BYTES WRITTEN: %lld\n", byteCounter);
    printf("TOTAL TIME: %02ld:%02ld.%03ld (min:sec.mili)\n", minutes, seconds, milliseconds);

    // Cleanups
    cleanUpFileInfo(fileInfos, fileInfoSize);
    free(threads);
    free(threadArgsArray);

    return 0;
}