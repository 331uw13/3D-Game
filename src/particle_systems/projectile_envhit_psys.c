#include "projectile_envhit_psys.h"
#include "../state/state.h"
#include "../util.h"

#include <raymath.h>
#include <stdio.h>


// PARTICLE UPDATE
void projectile_envhit_psys_update(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
){
    const float scale_duration = part->max_lifetime;
    if(part->scale < scale_duration) {
        part->scale += gst->dt;
    }

    float st = lerp(part->scale / scale_duration, 0.0, 20.0);

    Matrix scale_matrix = MatrixScale(st, st, st);
    Matrix translation = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    
    *part->transform = MatrixMultiply(scale_matrix, translation);
}



// PARTICLE INITIALIZATION
void projectile_envhit_psys_init(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        Color part_color,
        void* extradata, int has_extradata
){
    part->scale = 0.0;
    part->position = origin;
    part->color = part_color;
    part->max_lifetime = 0.35;
}


