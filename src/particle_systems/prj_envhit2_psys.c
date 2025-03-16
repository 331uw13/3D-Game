#include "prj_envhit2_psys.h"
#include "../state.h"
#include "../util.h"

#include <raymath.h>
#include <stdio.h>


// PARTICLE UPDATE
void prj_envhit_part2_psys_update(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
){

    const float scale_duration = 1.0;
    if(part->scale < scale_duration) {
        part->scale += gst->dt;
    }

    float st = lerp(part->scale / scale_duration, 3.0, 0.0);

    part->position = Vector3Add(part->position, Vector3Scale(part->velocity, gst->dt));


    Matrix scale_matrix = MatrixScale(st, st, st);
    Matrix translation = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    
    *part->transform = MatrixMultiply(scale_matrix, translation);
}



// PARTICLE INITIALIZATION
void prj_envhit_part2_psys_init(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        void* extradata, int has_extradata
){

    part->scale = 0.0;
    part->position = origin;
   
    const float r = 10.0;
    part->velocity = (Vector3) {
        RSEEDRANDOMF(-r, r),
        RSEEDRANDOMF(-r, r),
        RSEEDRANDOMF(-r, r)
    };

    *part->transform = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    part->max_lifetime = 1.0;
}


