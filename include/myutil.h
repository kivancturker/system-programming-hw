//
// Created by Kıvanç TÜRKER on 20.05.2024.
//

#ifndef MYUTIL_H
#define MYUTIL_H

#include <sys/types.h>
#include <errno.h>
#include <pthread.h>

#define NO_EINTR(expr) while ((expr) == -1 && errno == EINTR);

#define MAX_PATH_SIZE 512
#define MAX_DIR_PATH_SIZE 255
#define FILEIO_BUFFER_SIZE 1024

enum FileType {
    REGULAR_FILE,
    FIFO_FILE,
    DIRECTORY,
    UNKNOWN_FILE_TYPE
};

struct Args {
    int bufferSize;
    int threadCount;
    char srcPath[MAX_DIR_PATH_SIZE];
    char destPath[MAX_DIR_PATH_SIZE];
};

struct FileInfo {
    int srcFd;
    int destFd;
    char filename[MAX_PATH_SIZE];
    char srcFilePath[MAX_PATH_SIZE];
    char destFilePath[MAX_PATH_SIZE];
    enum FileType type;
};

struct FileStats {
    int regularFileCount;
    int fifoCount;
    int directoryCount;
    off_t totalBytes;
};

struct ThreadArgs {
    struct Queue *bufferQueue;
    pthread_mutex_t *bufferMutex;
    pthread_mutex_t *byteCounterMutex;
    pthread_mutex_t *terminationMutex;
    pthread_mutex_t *barrierMutex;
    pthread_cond_t* bufferNotEmptyCond;
    pthread_cond_t* bufferNotFullCond;
    pthread_cond_t *terminationCond;
    pthread_cond_t *barrierCond;
    char destPath[MAX_DIR_PATH_SIZE];
    char srcPath[MAX_DIR_PATH_SIZE];
    off_t* byteCounter;
    int* isFinished;
    int* barrierArrival;
    struct FileInfo* fileInfos;
    int fileInfoSize;
    int workerCount;
};


void parseArgs(int argc, char *argv[], struct Args *args);
void prepareDirectory(const char* dirPath);
int removeItem(const char *path);
int joinAllThreads(pthread_t* threads, int threadCount);
int cancelAllThreads(pthread_t* threads, int threadCount);
int workerPoolCreation();
void arriveBarrier(pthread_mutex_t* barrierMutex, pthread_cond_t* barrierCond, int* barrierArrival, int workerCount);

#endif // MYUTIL_H
