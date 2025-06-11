#ifndef LIGHT_H
#define LIGHT_H

#include <raylib.h>
#include <stddef.h>
#include "lib/glad.h"
// Modified from raylibs 'rlights'


#define MAX_TMP_LIGHTS 32


#define GLSL_LIGHT_STRUCT_SIZE (4*4 + 4*4 + 4*4)
    
// NOTE: Light updates happen in 'src/chunk.c' chunk_update_lights().


struct state_t;
struct chunk_t;


struct light_t {
    Color   color;
    Vector3 position;
    float   strength;
    float   radius;

    // The current chunk light is in.
    struct chunk_t* chunk;

    int index;
    uint8_t  enabled;
};


int   add_light     (struct state_t* gst, struct chunk_t* chunk, struct light_t* light);
void  remove_light  (struct state_t* gst, struct light_t* light);


#endif
