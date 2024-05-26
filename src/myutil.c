//
// Created by Kıvanç TÜRKER on 20.05.2024.
//
#include "myutil.h"
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>

void parseArgs(int argc, char *argv[], struct Args *args) {
    // Parse the args and put them into the struct
    args->bufferSize = atoi(argv[1]);
    args->threadCount = atoi(argv[2]);
    strncpy(args->srcPath, argv[3], MAX_DIR_PATH_SIZE);
    strncpy(args->destPath, argv[4], MAX_DIR_PATH_SIZE);
}

void prepareDirectory(const char* dirPath) {
    // Delete the directory and all its contents
    struct stat st;
    // First check if the directory exists
    if (stat(dirPath, &st) == 0) {
        if (removeItem(dirPath) == -1) {
            perror("removeItem");
        }
    }

    // Create the directory
    if (mkdir(dirPath, 0755) == -1) {
        perror("Error creating directory");
    }
}

int removeItem(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) == -1) {
        perror("stat");
        return -1;
    }
    if (S_ISDIR(statbuf.st_mode)) {
        DIR *d = opendir(path);
        if (d == NULL) {
            perror("opendir");
            return -1;
        }
        struct dirent *dir;
        while ((dir = readdir(d)) != NULL) {
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) {
                continue;
            }
            char buf[512];
            snprintf(buf, sizeof(buf), "%s/%s", path, dir->d_name);
            if (removeItem(buf) == -1) {
                closedir(d);
                return -1;
            }
        }
        closedir(d);
        if (rmdir(path) == -1) {
            perror("rmdir");
            return -1;
        }
    } else {
        if (remove(path) == -1) {
            perror("remove");
            return -1;
        }
    }
    return 0;
}

int joinAllThreads(pthread_t* threads, int threadCount) {
    for (int i = 0; i < threadCount; i++) {
        int joinResult = pthread_join(threads[i], NULL);
        if (joinResult != 0) {
            fprintf(stderr, "Error: pthread_join failed with error number %d\n", joinResult);
            return -1;
        }
    }
    return 0;
}

int cancelAllThreads(pthread_t* threads, int threadCount) {
    for (int i = 0; i < threadCount; i++) {
        int cancelResult = pthread_cancel(threads[i]);
        if (cancelResult != 0) {
            fprintf(stderr, "Error: pthread_cancel failed with error number %d\n", cancelResult);
            return -1;
        }
    }
    return 0;
}