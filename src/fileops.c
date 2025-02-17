//
// Created by Kıvanç TÜRKER on 20.05.2024.
//

#include "fileops.h"
#include "myutil.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


struct FileInfo* openAllFiles(const char* srcPath, const char* destPath, int* size) {
    int capacity = 10; // Initial capacity for array
    int count = 0; // Number of files found
    struct FileInfo* fileInfos = malloc(capacity * sizeof(struct FileInfo));
    if (fileInfos == NULL) {
        perror("Unable to allocate memory for fileInfos");
        return NULL;
    }

    traverseAndOpenFiles(srcPath, destPath, &fileInfos, &count, &capacity);

    // Resize array to final size
    fileInfos = realloc(fileInfos, count * sizeof(struct FileInfo));
    if (fileInfos == NULL) {
        perror("Unable to reallocate memory for fileInfos");
        return NULL;
    }

    *size = count;
    return fileInfos;
}

void traverseAndOpenFiles(const char* srcPath, const char* destPath, struct FileInfo** fileInfos, int* count, int* capacity) {
    DIR *dir;
    struct dirent *entry;
    struct stat fileStat;

    dir = opendir(srcPath);
    if (dir == NULL) {
        perror("Unable to open directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        char srcFilePath[MAX_PATH_SIZE];
        char destFilePath[MAX_PATH_SIZE];
        snprintf(srcFilePath, sizeof(srcFilePath), "%s/%s", srcPath, entry->d_name);
        snprintf(destFilePath, sizeof(destFilePath), "%s/%s", destPath, entry->d_name);
        enum FileType type = UNKNOWN_FILE_TYPE;
        int destFd = -1;
        int srcFd = -1;

        if (stat(srcFilePath, &fileStat) < 0) {
            fprintf(stderr, "Error stating file: %s ", srcFilePath);
            perror("Error");
            continue;
        }

        if (S_ISREG(fileStat.st_mode)) {
            /*
            destFd = open(destFilePath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (destFd == -1) {
                perror("Error opening destination file");
                continue;
            }
            srcFd = open(srcFilePath, O_RDONLY, 0666);
            if (srcFd == -1) {
                perror("Error opening source file");
                close(destFd);
                continue;
            }
            */
            type = REGULAR_FILE;
        } else if (S_ISFIFO(fileStat.st_mode)) {
            /*
            if (mkfifo(destFilePath, 0666) != 0) {
                perror("Error creating FIFO");
                continue;
            }
            */
            type = FIFO_FILE;
        } else if (S_ISDIR(fileStat.st_mode)) {
            if (mkdir(destFilePath, 0755) != 0 && errno != EEXIST) {
                perror("Error creating directory");
                continue;
            }
            type = DIRECTORY;
            // Recursively traverse the subdirectory
            traverseAndOpenFiles(srcFilePath, destFilePath, fileInfos, count, capacity);
        }

        // Resize array if necessary
        if (*count == *capacity) {
            *capacity *= 2;
            struct FileInfo* temp = realloc(*fileInfos, *capacity * sizeof(struct FileInfo));
            if (temp == NULL) {
                perror("Unable to reallocate memory for fileInfos");
                closedir(dir);
                return;
            }
            *fileInfos = temp;
        }

        // Add file descriptor and name to array
        (*fileInfos)[*count].destFd = destFd;
        (*fileInfos)[*count].srcFd = srcFd;
        strncpy((*fileInfos)[*count].filename, entry->d_name, MAX_PATH_SIZE);
        strncpy((*fileInfos)[*count].destFilePath, destFilePath, MAX_PATH_SIZE);
        strncpy((*fileInfos)[*count].srcFilePath, srcFilePath, MAX_PATH_SIZE);
        (*fileInfos)[*count].type = type;
        (*count)++;
    }

    closedir(dir);
}


void cleanUpFileInfo(struct FileInfo* fileInfos, int size) {
    for (int i = 0; i < size; i++) {
        if (fileInfos[i].destFd >= 0) {
            if (close(fileInfos[i].destFd) == -1) {
                if (errno != EBADF) {
                    perror("Error closing file");
                }
            }
        }
        if (fileInfos[i].srcFd >= 0) {
            if (close(fileInfos[i].srcFd) == -1) {
                if (errno != EBADF) {
                    perror("Error closing file");
                }
            }
        }
    }

    free(fileInfos);
}

void traverseDirectoryAndFillStats(const char* path, struct FileStats* stats) {
    DIR *dir;
    struct dirent *entry;
    struct stat fileStat;

    dir = opendir(path);
    if (dir == NULL) {
        perror("Unable to open directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        char filePath[MAX_PATH_SIZE];
        snprintf(filePath, sizeof(filePath), "%s/%s", path, entry->d_name);

        if (stat(filePath, &fileStat) < 0) {
            fprintf(stderr, "Error stating file: %s ", filePath);
            perror("Error");
            continue;
        }

        if (S_ISREG(fileStat.st_mode)) {
            stats->regularFileCount++;
            stats->totalBytes += fileStat.st_size;
        } else if (S_ISFIFO(fileStat.st_mode)) {
            stats->fifoCount++;
        } else if (S_ISDIR(fileStat.st_mode)) {
            stats->directoryCount++;
            // Recursively traverse the subdirectory
            traverseDirectoryAndFillStats(filePath, stats);
        }
    }

    closedir(dir);
}

int openRegularFiles(struct FileInfo* fileInfo) {
    fileInfo->destFd = open(fileInfo->destFilePath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fileInfo->destFd  == -1) {
        perror("Error opening destination file");
        return -1;
    }
    fileInfo->srcFd = open(fileInfo->srcFilePath, O_RDONLY, 0666);
    if (fileInfo->srcFd == -1) {
        perror("Error opening source file");
        close(fileInfo->destFd);
        return -1;
    }
    return 0;
}

int openFifoFiles(struct FileInfo fileInfo) {
    if (mkfifo(fileInfo.destFilePath, 0666) != 0) {
        perror("Error creating FIFO");
        return -1;
    }
    return 0;
}