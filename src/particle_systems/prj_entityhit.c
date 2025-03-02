#include "prj_envhit.h"
#include "../state.h"
#include "../util.h"

#include <raymath.h>
#include <stdio.h>


// PARTICLE UPDATE
void projectile_entityhit_psystem_pupdate(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
){

    part->position = Vector3Add(part->position, Vector3Scale(part->velocity, gst->dt));

    float ntime = normalize(part->lifetime, 0.0, part->max_lifetime);
    float st = lerp(ntime,  1.0, 0.0);

    Matrix scale_m = MatrixScale(st, st, st);
    Matrix position_m = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    
    position_m = MatrixMultiply(scale_m, position_m);
    *part->transform = position_m;

    part->velocity = Vector3Add(part->velocity, Vector3Scale(part->accel, gst->dt));


}



// PARTICLE INITIALIZATION
void projectile_entityhit_psystem_pinit(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        void* extradata, int has_extradata
){

    part->position = origin;
    part->velocity = Vector3Negate(velocity);
    part->velocity = Vector3Scale(part->velocity, 10.0);

    part->accel.y = -6.0;

    const float vr = 10.0;
    part->velocity.x += RSEEDRANDOMF(-vr, vr);
    part->velocity.y += RSEEDRANDOMF(-vr, vr);
    part->velocity.z += RSEEDRANDOMF(-vr, vr);

    Matrix position_m = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    
    *part->transform = position_m;
    part->max_lifetime = RSEEDRANDOMF(0.1, 0.8);
}


