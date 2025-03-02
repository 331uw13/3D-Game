#ifndef PRJ_ELVL0_ENVHIT_PSYSTEM_H
#define PRJ_ELVL0_ENVHIT_PSYSTEM_H

// * PROJECTILE_ENVHIT_PSYSTEM


#include <raylib.h>

struct psystem_t;
struct state_t;
struct particle_t;


// PARTICLE UPDATE
void projectile_elvl0_envhit_psystem_pupdate(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
);



// PARTICLE INITIALIZATION
void projectile_elvl0_envhit_psystem_pinit(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        void* extradata, int has_extradata
);



#endif
