#ifndef BERRY_COLLECT_H
#define BERRY_COLLECT_H

#include <raylib.h>

struct state_t;
struct psystem_t;
struct particle_t;

// PARTICLE UPDATE
void berry_collect_psys_update(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
);

// PARTICLE INITIALIZATION
void berry_collect_psys_init(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        Color part_color,
        void* extradata, int has_extradata
);


// For 'psystem->user_p' array index.
//#define BERRY_COLLECT_PSYS_FRACTAL_PTR_I 0


#endif
