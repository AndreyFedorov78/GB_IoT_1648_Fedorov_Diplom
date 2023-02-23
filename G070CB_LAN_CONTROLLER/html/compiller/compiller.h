//
// Created by Andrei Fedorov on 16.11.2022.
//

#ifndef G070CB_LAN_CONTROLLER_COMPILLER_H
#define G070CB_LAN_CONTROLLER_COMPILLER_H
#include <stdio.h>
#include <stdint.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

typedef struct fileInfo {
    char name[20];
    uint32_t address;
} fileInfo;

typedef struct fileList {
    fileInfo * info;
    struct fileList * next;
} fileList;


#endif //G070CB_LAN_CONTROLLER_COMPILLER_H
