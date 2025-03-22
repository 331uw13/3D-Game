#include "explosion_part3_psys.h"
#include "../state.h"
#include "../util.h"

#include <raymath.h>
#include <stdio.h>


// PARTICLE UPDATE
void explosion_part3_psys_update(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
){

    part->position = Vector3Add(part->position, Vector3Scale(part->velocity, gst->dt));

    float st = lerp(part->scale / part->max_lifetime, 10.0, 0.0);
    Matrix scale_matrix = MatrixScale(st, st, st);
    Matrix translation = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    
    if(part->scale < part->max_lifetime) {
        part->scale += gst->dt;
    }

    *part->transform = MatrixMultiply(scale_matrix, translation);
}

// PARTICLE INITIALIZATION
void explosion_part3_psys_init(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        void* extradata, int has_extradata
){

    part->position = origin;
    part->position.y += 5.0;

    const float rad = 5.0;
    part->position.x += rad*(cos(part->index) * sin(part->index));
    part->position.y += rad*sin(part->index);
    part->position.z += rad*(cos(part->index) * cos(part->index));

    const float v_r = 10.0;
    part->velocity = (Vector3) {
        RSEEDRANDOMF(-v_r, v_r),
        RSEEDRANDOMF(v_r*0.5, v_r*3.5),
        RSEEDRANDOMF(-v_r, v_r)
    };

    part->scale = 0.0; // Scale linear interpolation
    *part->transform = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    part->max_lifetime = RSEEDRANDOMF(2.0, 5.0);
    part->lifetime = 0.0;
}


