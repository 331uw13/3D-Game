#ifndef WATER_SPLASH_PSYSTEM_H
#define WATER_SPLASH_PSYSTEM_H



#include <raylib.h>

struct psystem_t;
struct state_t;
struct particle_t;


// When projectiles hit environment.



// PARTICLE UPDATE
void water_splash_psys_update(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
);


// PARTICLE INITIALIZATION
void water_splash_psys_init(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        void* extradata, int has_extradata
);



#endif
