#ifndef LIGHT_H
#define LIGHT_H

#include <raylib.h>
#include <stddef.h>
#include "lib/glad.h"
// Modified from raylibs 'rlights'


// NOTE: these values must be the same as in 'res/shaders/light.glsl'
#define MAX_PROJECTILE_LIGHTS 128
#define MAX_NORMAL_LIGHTS 16
#define MAX_DECAY_LIGHTS 32

// Light types.
#define LIGHT_DIRECTIONAL 0
#define LIGHT_POINT 1


// Static lights in lights ubo
#define SUN_LIGHT_ID 0
#define PLAYER_GUN_LIGHT_ID 1
#define INVENTORY_LIGHT_ID 2
#define MAX_STATIC_LIGHTS 3



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
