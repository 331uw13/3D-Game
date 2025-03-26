#ifndef EXPLOSION_PSYSTEM_H
#define EXPLOSION_PSYSTEM_H



#include <raylib.h>

struct psystem_t;
struct state_t;
struct particle_t;


#define PART_IDB_EXPLOSION 0
#define PART_IDB_SMOKE 1

// ------ Explosion PART 1 -----------

// PARTICLE UPDATE
void explosion_psys_update(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
);


// PARTICLE INITIALIZATION
void explosion_psys_init(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        Color part_color,
        void* extradata, int has_extradata
);


#endif
