#ifndef ENEMY_GUN_FX_PSYSTEM_H
#define ENEMY_GUN_FX_PSYSTEM_H



#include <raylib.h>

struct psystem_t;
struct state_t;
struct particle_t;


// PARTICLE UPDATE
void enemy_gunfx_psys_update(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
);


// PARTICLE INITIALIZATION
void enemy_gunfx_psys_init(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        void* extradata, int has_extradata
);



#endif
