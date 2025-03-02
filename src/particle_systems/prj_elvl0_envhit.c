#include "prj_elvl0_envhit.h"
#include "../state.h"
#include "../util.h"

#include <raymath.h>
#include <stdio.h>


// PARTICLE UPDATE
void projectile_elvl0_envhit_psystem_pupdate(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
){

    float ntime = normalize(part->lifetime*3.0, 0.0, part->max_lifetime);
    ntime *= 2;
    float st = lerp(ntime,  0.0, 1.0);


    Matrix scale_m = MatrixScale(st, st, st);
    Matrix position_m = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    
    position_m = MatrixMultiply(scale_m, position_m);

    *part->transform = position_m;
}



// PARTICLE INITIALIZATION
void projectile_elvl0_envhit_psystem_pinit(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        void* extradata, int has_extradata
){

    part->position = origin;
    Matrix position_m = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    
    *part->transform = position_m;
    part->max_lifetime = RSEEDRANDOMF(0.075, 0.15);
}


