#ifndef CUSTOM_GLSL_PREPROCESSOR_H
#define CUSTOM_GLSL_PREPROCESSOR_H

// https://github.com/331uw13/CustomGLSLPreprocessor

#include <stddef.h>

#define FILEOK 1

struct file_t {
    char*   data;
    size_t  size;
    int     ok;
};

int   read_file(struct file_t* file, const char* filename);
void  close_file(struct file_t* file);


// NOTE: if null is not returned the memory must be freed after use!
char* preproc_glsl(struct file_t* file, size_t* size_out);



#endif
