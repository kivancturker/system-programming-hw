//
// Created by Kıvanç TÜRKER on 20.05.2024.
//
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "myutil.h"
#include "fileops.h"

// Main with arguments
int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Usage: %s <bufferSize> <threadCount> <srcPath> <destPath>\n", argv[0]);
        return 1;
    }
    struct Args args;
    parseArgs(argc, argv, &args);

    int fileInfoSize = 0;
    struct FileInfo* fileInfos = openAllFiles(args.srcPath, args.destPath, &fileInfoSize);
    if (fileInfos == NULL) {
        return 1;
    }

    struct FileStats fileStats;
    traverseDirectoryAndFillStats(args.srcPath, &fileStats);

    printf("\n---------------STATISTICS--------------------\n");
    printf("Consumers: %d - Buffer Size: %d\n", args.threadCount, args.bufferSize);
    printf("Number of Regular File: %d\n", fileStats.regularFileCount);
    printf("Number of FIFO File: %d\n", fileStats.fifoCount);
    printf("Number of Directory: %d\n", fileStats.directoryCount);
    printf("TOTAL BYTES: %lld\n", fileStats.totalBytes);

    // Cleanups
    cleanUpFileInfo(fileInfos, fileInfoSize);

    return 0;
}