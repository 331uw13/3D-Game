#ifndef LIGHT_H
#define LIGHT_H

#include <raylib.h>
#include <stddef.h>
#include "lib/glad.h"
// Modified from raylibs 'rlights'


// NOTE: these 2 values must be the same as in 'res/shaders/light.glsl'
#define MAX_PROJECTILE_LIGHTS 64
#define MAX_NORMAL_LIGHTS 4


// Shader uniform locations index.
#define LIGHT_ENABLED_LOC 0
#define LIGHT_TYPE_LOC 1
#define LIGHT_POSITION_LOC 2
#define LIGHT_COLOR_LOC 3
#define LIGHT_STRENGTH_LOC 4
#define MAX_LIGHT_LOCS 5

// Light types.
#define LIGHT_DIRECTIONAL 0
#define LIGHT_POINT 1


struct state_t;

#define LIGHT_SHADER_STRUCT_SIZE (4*4 + 4*4 + 4*4 + 4*4)

struct light_t {
    int type;
    int enabled;
    Vector3 position;
    Color   color;
    float   strength;
   
    int index; // Index in lights or projectile lights uniform block array.
};


void set_light(
        struct state_t* gst,
        struct light_t* light,
        unsigned int ubo
        );

void disable_light(struct state_t* gst, struct light_t* light, unsigned int ubo);


#endif
