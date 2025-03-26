#ifndef LIGHT_H
#define LIGHT_H

#include <raylib.h>
#include <stddef.h>
#include "lib/glad.h"
// Modified from raylibs 'rlights'


// NOTE: these 2 values must be the same as in 'res/shaders/light.glsl'
#define MAX_PROJECTILE_LIGHTS 128
#define MAX_NORMAL_LIGHTS 16


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


struct light_t {
    int type;
    int ubo_index;
    int enabled;
    Vector3 position;
    Color   color;
    float   strength;
    float   radius;
    int     index; // Index in lights or projectile lights uniform block array.

    // How fast the light dims overtime if added to 'state->decay_lights' array?
    float decay; 
};


void set_light(
        struct state_t* gst,
        struct light_t* light,
        int ubo_index
        );

void disable_light(struct state_t* gst, struct light_t* light, int ubo_index);

void add_decay_light(struct state_t* gst, struct light_t* light, float decay_time_mult);
void update_decay_lights(struct state_t* gst);


#endif
