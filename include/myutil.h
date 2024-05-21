//
// Created by Kıvanç TÜRKER on 20.05.2024.
//

#ifndef MYUTIL_H
#define MYUTIL_H

#include <sys/types.h>

#define NO_EINTR(expr) while ((expr) == -1 && errno == EINTR);

#define MAX_PATH_SIZE 512
#define MAX_DIR_PATH_SIZE 255

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
    int fd;
    char name[MAX_PATH_SIZE];
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
    pthread_cond_t *bufferCond;
    char destPath[MAX_DIR_PATH_SIZE];
    char srcPath[MAX_DIR_PATH_SIZE];
    int* isFinished;
};


void parseArgs(int argc, char *argv[], struct Args *args);

#endif // MYUTIL_H
