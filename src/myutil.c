//
// Created by Kıvanç TÜRKER on 20.05.2024.
//
#include "myutil.h"
#include <string.h>
#include <stdlib.h>

void parseArgs(int argc, char *argv[], struct Args *args) {
    // Parse the args and put them into the struct
    args->bufferSize = atoi(argv[1]);
    args->threadCount = atoi(argv[2]);
    strncpy(args->srcPath, argv[3], MAX_DIR_PATH_SIZE);
    strncpy(args->destPath, argv[4], MAX_DIR_PATH_SIZE);
}