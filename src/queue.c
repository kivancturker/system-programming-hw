//
// Created by Kıvanç TÜRKER on 21.05.2024.
//

#include "queue.h"

#include <stdlib.h>

int initQueue(struct Queue* queue, int capacity) {
    if (queue == NULL) {
        return -1; // Invalid queue pointer
    }
    if (capacity > QUEUE_CAPACITY) {
        return -1; // Capacity is too large
    }
    queue->frontIndex = 0;
    queue->rearIndex = 0;
    queue->size = 0;
    queue->capacity = capacity;
    return 0;
}

int enqueue(struct Queue* queue, struct FileInfo fileInfo) {
    if (queue == NULL) {
        return -1; // Invalid queue pointer
    }
    if (queue->size >= queue->capacity) {
        return -1; // Queue is full
    }
    queue->fileInfo[queue->rearIndex] = fileInfo;
    queue->rearIndex = (queue->rearIndex + 1) % queue->capacity;
    queue->size++;
    return 0; // Enqueue operation successful
}

int dequeue(struct Queue* queue, struct FileInfo* fileInfo) {
    if (queue == NULL || fileInfo == NULL) {
        return -1; // Invalid queue or connectionRequest pointer
    }
    if (queue->size <= 0) {
        return -1; // Queue is empty
    }
    *fileInfo = queue->fileInfo[queue->frontIndex];
    queue->frontIndex = (queue->frontIndex + 1) % queue->capacity;
    queue->size--;
    return 0; // Dequeue operation successful
}

int isQueueEmpty(struct Queue* queue) {
    return (queue->size == 0);
}

int isQueueFull(struct Queue* queue) {
    return (queue->size == queue->capacity);
}
