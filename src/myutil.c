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
    if (nftw(dirPath, removeItem, 64, FTW_DEPTH | FTW_PHYS) < 0) {
        perror("nftw");
    }

    // Create the directory
    if (mkdir(dirPath, 0755) == -1) {
        perror("Error creating directory");
    }
}

int removeItem(const char *path, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
    if (remove(path) < 0) {
        perror("remove");
        return -1;
    }
    return 0;
}
