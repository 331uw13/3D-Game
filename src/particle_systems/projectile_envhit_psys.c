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

    if(part->idb == PART_IDB_ENVHIT_CIRCLE) {

        float st = lerp(part->scale / scale_duration, 0.0, 30.0);
        float alpha = lerp(normalize(part->lifetime, 0, part->max_lifetime), 1.0, 0.0);

        alpha = 1.0-pow(1.0-alpha, 5.0);
        part->color.a = alpha * 255;

        scale_matrix = MatrixScale(st, st, st);
        translation = MatrixTranslate(part->position.x, part->position.y, part->position.z); 
    }
    else
    if(part->idb == PART_IDB_ENVHIT_EFFECT) {
       
        part->position = Vector3Add(part->position, Vector3Scale(part->velocity, gst->dt));
        part->velocity = Vector3Scale(part->velocity, pow(0.9995, gst->dt*500));


        float st = lerp(part->scale / scale_duration, 8.0, 0.0);
        scale_matrix = MatrixScale(st, st, st);
        translation = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    }
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
        
    
    if(part->idb == PART_IDB_ENVHIT_CIRCLE) {
        part->scale = 0.0;
        part->max_lifetime = 0.375;
    }
    else
    if(part->idb == PART_IDB_ENVHIT_EFFECT) {
        part->scale = 0.0;
        part->position.x += RSEEDRANDOMF(-15.0, 15.0);
        part->position.y += RSEEDRANDOMF(-15.0, 15.0);
        part->position.z += RSEEDRANDOMF(-15.0, 15.0);
        part->velocity = Vector3Scale(velocity, 70.0);
        part->velocity.x += RSEEDRANDOMF(-40.0, 40.0);
        part->velocity.y += RSEEDRANDOMF(-40.0, 40.0);
        part->velocity.z += RSEEDRANDOMF(-40.0, 40.0);
        part->max_lifetime = RSEEDRANDOMF(0.4, 1.5);
    }

}


