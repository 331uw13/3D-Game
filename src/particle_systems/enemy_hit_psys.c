#include "enemy_hit_psys.h"
#include "../state.h"
#include "../util.h"

#include <raymath.h>
#include <stdio.h>


// PARTICLE UPDATE
void enemy_hit_psys_update(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
){

    float ntime = normalize(part->lifetime, 0.0, part->max_lifetime);
    float st = lerp(ntime,  1.0, 0.0);

    part->position = Vector3Add(part->position, Vector3Scale(part->velocity, gst->dt*40));

    Matrix scale_m = MatrixScale(st, st, st);
    Matrix transform = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    
    transform = MatrixMultiply(scale_m, transform);

    *part->transform = transform;
}



// PARTICLE INITIALIZATION
void enemy_hit_psys_init(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        void* extradata, int has_extradata
){

    part->position = origin;
    Matrix transform = MatrixTranslate(part->position.x, part->position.y, part->position.z);
   
    part->velocity = Vector3Normalize(Vector3Negate(velocity));
    
    const float r = 0.5;
    part->velocity.x += RSEEDRANDOMF(-r, r);
    part->velocity.y += RSEEDRANDOMF(-r, r);
    part->velocity.z += RSEEDRANDOMF(-r, r);
    

    *part->transform = transform;
    part->max_lifetime = RSEEDRANDOMF(0.485, 0.65);
}


