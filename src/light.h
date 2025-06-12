#ifndef LIGHT_H
#define LIGHT_H

#include <raylib.h>
#include <stddef.h>
#include "lib/glad.h"


#define GLSL_LIGHT_STRUCT_SIZE (4*4 + 4*4 + 4*4)

#define MAX_LIGHTS 256


struct state_t;

struct light_t {
    Color   color;
    Vector3 position;
    float   strength;
    float   radius;

    uint16_t index; // Index to 'state.lights' array. (Do not modify!)
    uint8_t enabled;
    uint8_t preserve; // If this is set. Light cant be overwritten even if its disabled.
};

#define NEVER_OVERWRITE 0  // No permission to overwrite enabled lights if array is full.
#define ALLOW_OVERWRITE 1  // May overwrite existing light data.


// IMPORTANT NOTES:
// 'add_light()' will return NULL if
// 'can_overwrite' is set to NEVER_OVERWRITE and
// there are no room for a new light.

// If 'light.preserve' is set to positive number
// it may still be overwritten if ALLOW_OVERWRITE is chosen.

// 'ALLOW_OVERWRITE' is guranteed to return a pointer.


struct light_t* add_light     (struct state_t* gst, struct light_t light_settings, int can_overwrite);
void            remove_light  (struct state_t* gst, struct light_t* light);





#endif
