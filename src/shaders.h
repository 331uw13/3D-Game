#ifndef SHADERS_H
#define SHADERS_H

#include <raylib.h>

struct state_t;


// Preprocess fragment shader.
int load_shader(const char* vs_filename, const char* fs_filename, Shader* shader);


void setup_all_shaders(struct state_t* gst);




#endif
