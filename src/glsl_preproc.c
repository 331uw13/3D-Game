#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "glsl_preproc.h"

// https://github.com/331uw13/CustomGLSLPreprocessor


int read_file(struct file_t* file, const char* filename) {
    int result = 0;

    file->data = NULL;
    file->size = 0;
    file->ok = 0;
    
    if(access(filename, F_OK)) {
        fprintf(stderr, "\033[31m(ERROR) '%s': File \"%s\" Not Found!\033[0m\n",
                __func__, filename);
        goto error;
    }


    int fd = open(filename, O_RDONLY);
    if(fd <= 0) {
        fprintf(stderr, "\033[31m(ERROR) '%s': (open) \"%s\" %s\033[0m\n",
                __func__, filename, strerror(errno));
        goto error;
    }

    struct stat sb;    
    if(fstat(fd, &sb) == -1) {
        fprintf(stderr, "\033[31m(ERROR) '%s': (fstat) \"%s\" %s\033[0m\n",
                __func__, filename, strerror(errno));
        goto error_and_close;
    }


    char* ptr = NULL;
    ptr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if(!ptr) {
        fprintf(stderr, "\033[31m(ERROR) '%s': (mmap) \"%s\" %s\033[0m\n",
                __func__, filename, strerror(errno));
        goto error_and_close;
    }

    printf("'%s' Read '%s'\n", __FILE__, filename);


    file->data = NULL;
    file->data = malloc(sb.st_size+1);
    memset(file->data, 0, sb.st_size+1);
    memmove(file->data, ptr, sb.st_size);


    //printf("\033[90m%s\033[0m\n", file->data);


    file->size = sb.st_size;
    file->ok = FILEOK;
    

    result = 1;
    munmap(ptr, sb.st_size);

error_and_close:
    close(fd);

error:
    return result;
}


void close_file(struct file_t* file) {
    if(!file) {
        return;
    }
    if(!file->data) {
        return;
    }

    free(file->data);
}


static char* copypaste_content(
        char* code, 
        size_t* code_size,
        size_t includetag_index,   // Begin of #include "..."
        size_t includetag_end_index, // End of #include "..."
        struct file_t* include_file
){

    // Allocate memory to hold both files content.
    char* new_data = NULL;
    const size_t new_size = *code_size + include_file->size;

    new_data = malloc(new_size);
    if(!new_data) {
        fprintf(stderr, "\033[31m(ERROR) '%s': (malloc) %s\033[0m\n",
                __func__, strerror(errno));
        goto error;
    }

    memset(new_data, 0, new_size);

    // Copy eveything at the begining of data until 'index' (where "#include" was found).

    memmove(
            new_data,
            code,
            includetag_index
            );


    // Copy 'include_file' content to index.

    memmove(
            new_data + includetag_index,
            include_file->data,
            include_file->size
            );

    // Copy rest of the file to 'index + include_file->size'.

    memmove(
            new_data + (includetag_index + include_file->size),
            code + includetag_end_index,
            *code_size - includetag_end_index
            );


    *code_size += include_file->size;

error:
    return new_data;
}


#define INCLUDETAG "#include"
#define PREPROC_TMPBUF_SIZE 64


char* preproc_glsl(struct file_t* file, size_t* size_out) {

    // Create copy of file's data to prevent double free.
    char* code = NULL;
    code = malloc(file->size);
    if(!code) {
        fprintf(stderr, "\033[31m(ERROR) '%s': (malloc) %s\033[0m\n",
                __func__, strerror(errno));
        goto error;
    }

    memmove(code, file->data, file->size);


    size_t code_size = file->size;

    char tmpbuf[PREPROC_TMPBUF_SIZE] = { 0 };
    size_t tmpbuf_i = 0;
    size_t includetag_index = 0; // Where "#include" was found
    int reading_totmp = 0;       // When copy into 'tmpbuf'
    int includetag_found = 0;
    int num_quatation_marks = 0; // Used to detect where '#include ("this is")'
    int lines = 0;  // How many newline characters

    for(size_t i = 0; i < code_size; i++) {
        char c = code[i];

        // Read from file data into 'tmpbuf' when "#" is found
        // until 'tmpbuf' matches "#include".
        if(!includetag_found) {
            if(c == '\n') {
                // Keep track of lines if errors occur.
                lines++;
                continue;
            }
            if(c == 0x20) {
discard_tmpbuf:
                reading_totmp = 0;
                memset(tmpbuf, 0, PREPROC_TMPBUF_SIZE);
                tmpbuf_i = 0;
                continue;
            }
            if(c == '#') {
                tmpbuf[0] = '#';
                tmpbuf_i = 1;
                reading_totmp = 1;
                includetag_index = i;
                continue;
            }


            if(reading_totmp) {
                if(tmpbuf_i >= PREPROC_TMPBUF_SIZE) {
                    goto discard_tmpbuf;
                }
                tmpbuf[tmpbuf_i] = c;
                tmpbuf_i++;
            
                if(strcmp(tmpbuf, INCLUDETAG) == 0) {
                    includetag_found = 1;
                    memset(tmpbuf, 0, PREPROC_TMPBUF_SIZE);
                    tmpbuf_i = 0;
                    num_quatation_marks = 0;
                    reading_totmp = 0;
                }
            }

            continue;
        }


        // Include tag is found. continue to read filename.
        // Start to read into 'tmpbuf' when " is found until another
        // or PREPROC_TMPBUF_SIZE is reached.
        if(c == '\"') {
            num_quatation_marks++;
            if(num_quatation_marks == 1) {
                reading_totmp = 1;
                continue;
            }            


            // Another " character was found. try to read the file content
            // and copy paste it in.

            struct file_t include_file = { .data = NULL, .size = 0, .ok = 0 };
            if(!read_file(&include_file, tmpbuf)) {
                goto error;
            }


            char* ptr = copypaste_content(
                    code,       // Current code.
                    &code_size, // Report back the size too.
                    includetag_index, // Where #include "..." begins
                    i+1,            // Where #include "..." ends  +1 for last quatation mark.
                    &include_file
                    );
            if(!ptr) {
                goto error;
            }


            free(code);
            code = ptr;

            i -= includetag_index;

            close_file(&include_file);
    
            includetag_found = 0;
            reading_totmp = 0;
            memset(tmpbuf, 0, PREPROC_TMPBUF_SIZE);
            tmpbuf_i = 0;
            num_quatation_marks = 0;
        }

        if(reading_totmp) {
            tmpbuf[tmpbuf_i] = c;
            tmpbuf_i++;
            if(tmpbuf_i >= PREPROC_TMPBUF_SIZE) {
                fprintf(stderr, "\033[31m(ERROR) '%s': Too long value for '%s' at line %i\033[0m\n",
                        __func__, INCLUDETAG, lines+1);
                goto error;
            }
        }
    }

    code[code_size-1] = '\0';

error:
    return code;
}



