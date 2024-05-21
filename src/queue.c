//
// Created by Kıvanç TÜRKER on 21.05.2024.
//

#include "queue.h"

#include <stdlib.h>

// Initialize the queue
int initQueue(struct Queue* queue) {
    if (queue == NULL) {
        return -1; // Invalid queue pointer
    }
    queue->frontIndex = 0;
    queue->rearIndex = 0;
    queue->size = 0;
    return 0; // Queue initialized successfully
}

// Enqueue a connection request into the queue
int enqueue(struct Queue* queue, struct FileInfo fileInfo) {
    if (queue == NULL) {
        return -1; // Invalid queue pointer
    }
    if (queue->size >= QUEUE_CAPACITY) {
        return -1; // Queue is full
    }
    queue->fileInfo[queue->rearIndex] = fileInfo;
    queue->rearIndex = (queue->rearIndex + 1) % QUEUE_CAPACITY;
    queue->size++;
    return 0; // Enqueue operation successful
}

// Dequeue a connection request from the queue
int dequeue(struct Queue* queue, struct FileInfo* fileInfo) {
    if (queue == NULL || fileInfo == NULL) {
        return -1; // Invalid queue or connectionRequest pointer
    }
    if (queue->size <= 0) {
        return -1; // Queue is empty
    }
    *fileInfo = queue->fileInfo[queue->frontIndex];
    queue->frontIndex = (queue->frontIndex + 1) % QUEUE_CAPACITY;
    queue->size--;
    return 0; // Dequeue operation successful
}

// Check if the queue is empty
int isQueueEmpty(struct Queue* queue) {
    if (queue == NULL) {
        return -1; // Invalid queue pointer
    }
    return (queue->size == 0);
}
