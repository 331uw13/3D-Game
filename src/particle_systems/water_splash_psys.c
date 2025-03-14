#include "water_splash_psys.h"
#include "../state.h"
#include "../util.h"

#include <raymath.h>
#include <stdio.h>


// PARTICLE UPDATE
void water_splash_psys_update(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
){

    part->position = Vector3Add(part->position, Vector3Scale(part->velocity, gst->dt));

    float st = lerp(normalize(part->lifetime, 0.0, part->max_lifetime), 2.0, 0.0);
    Matrix scale_matrix = MatrixScale(st, st, st);
    Matrix translation = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    
    part->velocity.y -= (500*0.08) * gst->dt;

    *part->transform = MatrixMultiply(scale_matrix, translation);
}



// PARTICLE INITIALIZATION
void water_splash_psys_init(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        void* extradata, int has_extradata
){

    origin.x += 2.0*sin(part->index);
    origin.z += 2.0*cos(part->index);
    part->position = origin;
    Matrix transform = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    
    const float v_r = 10.0;
    part->velocity = (Vector3) {
        velocity.x*v_r*2 + RSEEDRANDOMF(-v_r, v_r),
        velocity.y*v_r + RSEEDRANDOMF(v_r, v_r*2),
        velocity.z*v_r*2 + RSEEDRANDOMF(-v_r, v_r)
    };

    *part->transform = transform;
    part->max_lifetime = RSEEDRANDOMF(0.3, 1.0);
}


