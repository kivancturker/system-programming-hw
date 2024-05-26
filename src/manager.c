//
// Created by Kıvanç TÜRKER on 26.05.2024.
//

#include "manager.h"
#include "myutil.h"
#include "fileops.h"
#include "queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void* manager(void* args) {
    struct ThreadArgs* threadArgs = (struct ThreadArgs*) args;

    pthread_mutex_lock(threadArgs->bufferMutex);
    struct Queue* bufferQueue = threadArgs->bufferQueue;
    pthread_mutex_t* bufferMutex = threadArgs->bufferMutex;
    pthread_cond_t* bufferCond = threadArgs->bufferCond;
    pthread_mutex_t* terminationMutex = threadArgs->terminationMutex;
    pthread_cond_t* terminationCond = threadArgs->terminationCond;
    char destPath[MAX_DIR_PATH_SIZE];
    char srcPath[MAX_DIR_PATH_SIZE];
    int* isFinished = threadArgs->isFinished;
    strncpy(destPath, threadArgs->destPath, MAX_DIR_PATH_SIZE);
    strncpy(srcPath, threadArgs->srcPath, MAX_DIR_PATH_SIZE);
    pthread_mutex_unlock(threadArgs->bufferMutex);

    int fileInfoSize = 0;
    struct FileInfo* fileInfos = openAllFiles(srcPath, destPath, &fileInfoSize);
    if (fileInfos == NULL) {
        return NULL;
    }

    // Send every file info to buffer queue
    for (int i = 0; i < fileInfoSize; i++) {
        if (fileInfos[i].type == REGULAR_FILE) {
            if (openRegularFiles(&fileInfos[i]) == -1) {
                perror("Erro while opening regular file");
            }
        } else if (fileInfos[i].type == FIFO_FILE) {
            if (openFifoFiles(fileInfos[i]) == -1) {
                perror("Error while opening fifo file");
            }
        }
        pthread_mutex_lock(bufferMutex);
        if (isQueueFull(bufferQueue)) {
            pthread_cond_wait(bufferCond, bufferMutex);
        }
        if (fileInfos[i].type != DIRECTORY) {
            enqueue(bufferQueue, fileInfos[i]);
            pthread_cond_broadcast(bufferCond);
        }
        pthread_mutex_unlock(bufferMutex);
    }

    // Wait until the queue is fully processed
    pthread_mutex_lock(bufferMutex);
    while (!isQueueEmpty(bufferQueue)) {
        pthread_cond_wait(bufferCond, bufferMutex);
    }
    *isFinished = 1;
    pthread_cond_broadcast(bufferCond);
    pthread_mutex_unlock(bufferMutex);

    pthread_mutex_lock(terminationMutex);
    pthread_cond_signal(terminationCond);
    pthread_mutex_unlock(terminationMutex);

    return NULL;
}