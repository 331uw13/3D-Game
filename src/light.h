#ifndef LIGHT_H
#define LIGHT_H

#include <raylib.h>
#include <stddef.h>
#include "lib/glad.h"
// Modified from raylibs 'rlights'


#define MAX_DECAY_LIGHTS 32


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

    // Set decaying to positive number 
    // if light has to fade out before it is removed completely.
    int   decay;
    float decay_timer;
    float decay_speed;

    uint16_t index;
};

// WARNING: May return NULL. if chunk cant add light.
struct light_t* add_light(struct state_t* gst, struct chunk_t* chunk, struct light_t light_settings);

void remove_light(struct state_t* gst, struct light_t* light);



#endif
