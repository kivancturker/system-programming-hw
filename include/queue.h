//
// Created by Kıvanç TÜRKER on 21.05.2024.
//

#ifndef QUEUE_H
#define QUEUE_H

#include "myutil.h"

#define QUEUE_CAPACITY 2048

struct Queue {
    int frontIndex;
    int rearIndex;
    int size;
    int capacity;
    struct FileInfo fileInfo[QUEUE_CAPACITY];
};

int initQueue(struct Queue* queue, int capacity);
int enqueue(struct Queue* queue, struct FileInfo fileInfo);
int dequeue(struct Queue* queue, struct FileInfo* fileInfo);
int isQueueEmpty(struct Queue* queue);

#endif //QUEUE_H
