//
// Created by Kıvanç TÜRKER on 20.05.2024.
//

#ifndef FILEOPS_H
#define FILEOPS_H

#include "myutil.h"

struct FileInfo* openAllFiles(const char* srcPath, const char* destPath, int* size);
void traverseAndOpenFiles(const char* srcPath, const char* destPath, struct FileInfo** fileInfos, int* count, int* capacity);
void cleanUpFileInfo(struct FileInfo* fileInfos, int size);
void traverseDirectoryAndFillStats(const char* path, struct FileStats* stats);
#endif //FILEOPS_H
