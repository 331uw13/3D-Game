#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "platform.h"

void platform_init_file(platform_file_t* file) {
    file->data = NULL;
    file->size = 0;
    file->ok = 0;
}

int platform_file_exists(const char* filename) {
    return (GetFileAttributes(filename) != INVALID_FILE_ATTRIBUTES);
}

int platform_read_file(platform_file_t* file, const char* filename) {
    int result = 0;

    file->data = NULL;
    file->size = 0;
    file->ok = 0;
    
    if (!platform_file_exists(filename)) {
        fprintf(stderr, "\033[31m(ERROR) '%s': File \"%s\" Not Found!\033[0m\n",
                __func__, filename);
        goto error;
    }

    HANDLE hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "\033[31m(ERROR) '%s': (CreateFile) \"%s\" Error: %lu\033[0m\n",
                __func__, filename, GetLastError());
        goto error;
    }

    LARGE_INTEGER fileSize;
    if(!GetFileSizeEx(hFile, &fileSize)) {
        fprintf(stderr, "\033[31m(ERROR) '%s': (GetFileSizeEx) \"%s\" Error: %lu\033[0m\n",
                __func__, filename, GetLastError());
        goto error_and_close;
    }

    HANDLE hMapping = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if(!hMapping) {
        fprintf(stderr, "\033[31m(ERROR) '%s': (CreateFileMapping) \"%s\" Error: %lu\033[0m\n",
                __func__, filename, GetLastError());
        goto error_and_close;
    }

    void* ptr = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
    if(!ptr) {
        fprintf(stderr, "\033[31m(ERROR) '%s': (MapViewOfFile) \"%s\" Error: %lu\033[0m\n",
                __func__, filename, GetLastError());
        goto error_and_mapping;
    }

    file->data = malloc(fileSize.QuadPart + 1);
    if(!file->data) {
        fprintf(stderr, "\033[31m(ERROR) '%s': (malloc) Failed to allocate memory\033[0m\n",
                __func__);
        goto error_and_unmap;
    }

    memset(file->data, 0, fileSize.QuadPart + 1);
    memmove(file->data, ptr, fileSize.QuadPart);

    file->size = fileSize.QuadPart;
    file->ok = 1; // Success
    result = 1;

error_and_unmap:
    UnmapViewOfFile(ptr);

error_and_mapping:
    CloseHandle(hMapping);

error_and_close:
    CloseHandle(hFile);

error:
    return result;
}

void platform_close_file(platform_file_t* file) {
    if(!file) {
        return;
    }
    if(!file->data) {
        return;
    }

    free(file->data);
    file->data = NULL;
    file->size = 0;
    file->ok = 0;
} 