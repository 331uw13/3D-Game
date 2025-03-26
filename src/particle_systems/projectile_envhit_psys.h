#ifndef PRJ_ENVHIT_PSYSTEM_H
#define PRJ_ENVHIT_PSYSTEM_H



#include <raylib.h>

struct psystem_t;
struct state_t;
struct particle_t;



// PARTICLE UPDATE
void projectile_envhit_psys_update(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
);


// PARTICLE INITIALIZATION
void projectile_envhit_psys_init(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        Color part_color,
        void* extradata, int has_extradata
);



#endif
