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
   
    Matrix scale_matrix;
    Matrix translation;


    part->position = Vector3Add(part->position, Vector3Scale(part->velocity, gst->dt));
    //part->velocity = Vector3Scale(part->velocity, pow(0.9995, gst->dt*GRAVITY_CONST));

    part->velocity = Vector3Scale(part->velocity, part->accel.x);
    part->accel.x *= pow(0.99999, gst->dt*GRAVITY_CONST);

    float st = lerp(part->scale / scale_duration, 8.0, 0.0);
    scale_matrix = MatrixScale(st, st, st);
    translation = MatrixTranslate(part->position.x, part->position.y, part->position.z);
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
    
    part->position = origin;
    part->color = part_color;
        
   
    part->velocity = Vector3Scale(velocity, 120.0);
    part->scale = 0.0;
    part->accel.x = 1.0;

    const float P = 5.0;
    part->position.x += RSEEDRANDOMF(-P, P);
    part->position.y += RSEEDRANDOMF(-P, P);
    part->position.z += RSEEDRANDOMF(-P, P);
    
    const float V = 100.0;
    part->velocity.x += RSEEDRANDOMF(-V, V);
    part->velocity.y += RSEEDRANDOMF(-V, V);
    part->velocity.z += RSEEDRANDOMF(-V, V);

    part->max_lifetime = RSEEDRANDOMF(0.2, 0.65);
}


