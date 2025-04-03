#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "platform.h"

void platform_init_file(platform_file_t* file) {
    file->data = NULL;
    file->size = 0;
    file->ok = 0;
}

int platform_file_exists(const char* filename) {
    return (access(filename, F_OK) == 0);
}

int platform_read_file(platform_file_t* file, const char* filename) {
    FILE* fp = NULL;
    long file_len = 0;
    size_t bytes_read = 0;

    file->data = NULL;
    file->size = 0;
    file->ok = 0;

    if (!platform_file_exists(filename)) {
        fprintf(stderr, "\033[31m(ERROR) '%s': File \"%s\" Not Found!\033[0m\n",
                __func__, filename);
        goto error;
    }

    fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "\033[31m(ERROR) '%s': (fopen) Failed to open \"%s\": %s\033[0m\n",
                __func__, filename, strerror(errno));
        goto error;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fprintf(stderr, "\033[31m(ERROR) '%s': (fseek) Failed to seek end of file \"%s\": %s\033[0m\n",
                __func__, filename, strerror(errno));
        goto error_and_close;
    }

    file_len = ftell(fp);
    if (file_len == -1L) {
        fprintf(stderr, "\033[31m(ERROR) '%s': (ftell) Failed to get size of file \"%s\": %s\033[0m\n",
                __func__, filename, strerror(errno));
        goto error_and_close;
    }
    file->size = (size_t)file_len;

    if (fseek(fp, 0, SEEK_SET) != 0) {
        fprintf(stderr, "\033[31m(ERROR) '%s': (fseek) Failed to seek beginning of file \"%s\": %s\033[0m\n",
                __func__, filename, strerror(errno));
        goto error_and_close;
    }

    file->data = malloc(file->size + 1);
    if (!file->data) {
        fprintf(stderr, "\033[31m(ERROR) '%s': (malloc) Failed to allocate %zu bytes for file \"%s\": %s\033[0m\n",
                __func__, file->size + 1, filename, strerror(errno));
        goto error_and_close;
    }

    bytes_read = fread(file->data, 1, file->size, fp);
    if (bytes_read != file->size) {
        if (ferror(fp)) {
            fprintf(stderr, "\033[31m(ERROR) '%s': (fread) Error reading file \"%s\": %s\033[0m\n",
                    __func__, filename, strerror(errno));
        } else {
             fprintf(stderr, "\033[31m(ERROR) '%s': (fread) Read %zu bytes, expected %zu for file \"%s\"\033[0m\n",
                    __func__, bytes_read, file->size, filename);
        }
        free(file->data);
        file->data = NULL;
        file->size = 0;
        goto error_and_close;
    }
    file->data[file->size] = '\0';

    file->ok = 1;
    fclose(fp);
    return 1;

error_and_close:
    if (fp) {
        fclose(fp);
    }
error:
    if(file->data) free(file->data);
    file->data = NULL;
    file->size = 0;
    file->ok = 0;
    return 0;
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