#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

typedef struct
{
    char accounts_file[256];
    char trace_file[256];
    int tick_ms;
    char deadlock_mode[20];
    bool verbose;
} AppConfig;

extern AppConfig config;

#endif