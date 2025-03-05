#ifndef LIGHT_H
#define LIGHT_H

#include <raylib.h>

// Modified from raylibs 'rlights'


// NOTE: these 2 values must be the same as in 'default' fragment shader code.
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


struct light_t {
    int type;
    int enabled;
    Vector3 position;
    Color   color;
    float   strength;
        
    // Uniform locations.
    int locs[MAX_LIGHT_LOCS];
};


// This one creates the light into 'state normal_lights' array.
void add_light(
        struct state_t* gst,
        int light_type,
        Vector3 position,
        Color color,
        Shader shader
        );


// This one create the light into where ever the 'lightptr' points to
void add_projectile_light(
        struct state_t* gst,
        struct light_t* lightptr,
        Vector3 position,
        Color color,
        Shader shader
        );


void update_light_values(struct light_t* light, Shader shader);
void disable_light(struct light_t* light, Shader shader);

#endif
