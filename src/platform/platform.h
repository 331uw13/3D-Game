#ifndef PLATFORM_H
#define PLATFORM_H

#include <stddef.h>

/**
 * Platform abstraction layer for file operations
 * Supports both Windows and Unix-like systems (Linux, macOS)
 */

typedef struct {
    char*   data;
    size_t  size;
    int     ok;
} platform_file_t;


void platform_init_file(platform_file_t* file);
int platform_read_file(platform_file_t* file, const char* filename);
void platform_close_file(platform_file_t* file);
int platform_file_exists(const char* filename);

#endif // PLATFORM_H
