#ifndef LIGHT_H
#define LIGHT_H

#include <raylib.h>

// Modified from raylibs 'rlights'


// NOTE: these 2 values must be the same as in shader code.
#define MAX_LIGHTS ( 64/* projectiles */ +  4/* normal lights */)
//#define MAX_PROJECTILE_LIGHTS 64


// Shader uniform locations index.
#define LIGHT_ENABLED_LOC 0
#define LIGHT_TYPE_LOC 1
#define LIGHT_POSITION_LOC 2
#define LIGHT_COLOR_LOC 3

#define MAX_LIGHT_LOCS 4

// Light types.
#define LIGHT_DIRECTIONAL 0
#define LIGHT_POINT 1


struct state_t;


struct light_t {
    int type;
    int enabled;
    Vector3 position;
    Color   color;

    // Uniform locations.
    int locs[MAX_LIGHT_LOCS];
};

// Returns pointer to just added light in 'gst->lights' array.
// may return NULL if max lights was reached.
void add_light(
        struct state_t* gst,
        struct light_t* light,
        int light_type,
        Vector3 position,
        Color color,
        Shader shader
        );

void update_light_values(struct light_t* light, Shader shader);
void disable_light(struct light_t* light, Shader shader);

#endif
