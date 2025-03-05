#include "prj_envhit_psys.h"
#include "../state.h"
#include "../util.h"

#include <raymath.h>
#include <stdio.h>


// PARTICLE UPDATE
void prj_envhit_psys_update(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
){

    float ntime = normalize(part->lifetime*3.0, 0.0, part->max_lifetime);
    ntime *= 2;
    float st = lerp(ntime,  0.0, 1.0);

    Matrix scale_m = MatrixScale(st, st, st);
    Matrix transform = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    
    transform = MatrixMultiply(scale_m, transform);
    *part->transform = transform;
}



// PARTICLE INITIALIZATION
void prj_envhit_psys_init(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        void* extradata, int has_extradata
){

    part->position = origin;
    Matrix transform = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    
    *part->transform = transform;
    part->max_lifetime = RSEEDRANDOMF(0.185, 0.25);
}


