#ifndef CUSTOM_GLSL_PREPROCESSOR_H
#define CUSTOM_GLSL_PREPROCESSOR_H

// https://github.com/331uw13/CustomGLSLPreprocessor

#include <stddef.h>
#include "platform.h"

// NOTE: if null is not returned the memory must be freed after use!
char* preproc_glsl(platform_file_t* file, size_t* size_out);

#endif
