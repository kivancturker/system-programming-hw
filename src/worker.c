//
// Created by Kıvanç TÜRKER on 21.05.2024.
//

#include "worker.h"
#include "myutil.h"
#include "queue.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void* worker(void* arg) {
    struct ThreadArgs* threadArgs = (struct ThreadArgs*) arg;

    pthread_mutex_lock(threadArgs->bufferMutex);
    struct Queue* bufferQueue = threadArgs->bufferQueue;
    pthread_mutex_t *bufferMutex = threadArgs->bufferMutex;
    pthread_mutex_t *byteCounterMutex = threadArgs->byteCounterMutex;
    pthread_cond_t *bufferCond = threadArgs->bufferCond;
    char destPath[MAX_DIR_PATH_SIZE];
    char srcPath[MAX_DIR_PATH_SIZE];
    int* isFinished = threadArgs->isFinished;
    off_t* byteCounter = threadArgs->byteCounter;
    strncpy(destPath, threadArgs->destPath, MAX_DIR_PATH_SIZE);
    strncpy(srcPath, threadArgs->srcPath, MAX_DIR_PATH_SIZE);
    pthread_mutex_unlock(threadArgs->bufferMutex);

    int bytesRead = 1;
    int bytesWritten = 0;
    char fileBuffer[FILEIO_BUFFER_SIZE];

    struct FileInfo fileInfo;

    while (!(*isFinished)) {
        pthread_mutex_lock(bufferMutex);
        while (isQueueEmpty(bufferQueue)) {
            pthread_cond_wait(bufferCond, bufferMutex);
            if (*isFinished) {
                pthread_mutex_unlock(bufferMutex);
                return NULL;
            }
        }
        dequeue(bufferQueue, &fileInfo);
        pthread_mutex_unlock(bufferMutex);
        pthread_cond_signal(bufferCond);

        bytesRead = 1;
        while(bytesRead > 0) {
            NO_EINTR(bytesRead = read(fileInfo.srcFd, fileBuffer, FILEIO_BUFFER_SIZE));
            if (bytesRead < 0) {
                perror("Error reading file");
                close(fileInfo.srcFd);
                close(fileInfo.destFd);
                return NULL;
            }
            NO_EINTR(bytesWritten = write(fileInfo.destFd, fileBuffer, bytesRead));
            if (bytesWritten < 0) {
                perror("Error writing file");
                close(fileInfo.srcFd);
                close(fileInfo.destFd);
                return NULL;
            }
            pthread_mutex_lock(byteCounterMutex);
            *byteCounter += bytesWritten;
            pthread_mutex_unlock(byteCounterMutex);
        }
        close(fileInfo.srcFd);
        close(fileInfo.destFd);
    }

    return NULL;
}
