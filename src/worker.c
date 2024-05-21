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
    pthread_cond_t *bufferCond = threadArgs->bufferCond;
    char destPath[MAX_DIR_PATH_SIZE];
    char srcPath[MAX_DIR_PATH_SIZE];
    int* isFinished = threadArgs->isFinished;
    strncpy(destPath, threadArgs->destPath, MAX_DIR_PATH_SIZE);
    strncpy(srcPath, threadArgs->srcPath, MAX_DIR_PATH_SIZE);
    pthread_mutex_unlock(threadArgs->bufferMutex);

    struct FileInfo fileInfo;

    while (!(*isFinished)) {
        // Take the item from buffer and sleep fore a while
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

        // Process the file
        printf("File %s is being processed by thread %p\n", fileInfo.name, pthread_self());
    }

    return NULL;
}
