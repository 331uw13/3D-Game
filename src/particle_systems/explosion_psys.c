#include "explosion_psys.h"
#include "../state/state.h"
#include "../util.h"

#include <raymath.h>
#include <stdio.h>



// ----- Explosion PART 1 ----------



// PARTICLE UPDATE
void explosion_psys_update(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
){
    Matrix translation = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    Matrix scale_matrix = MatrixIdentity();


    float scalelrp = lerp(part->n_lifetime, part->scale, 0.0);
    scale_matrix = MatrixScale(scalelrp, scalelrp, scalelrp);

    Vector3 vadd = Vector3Scale(part->velocity, gst->dt);
    part->position = Vector3Add(part->position, vadd);
    part->velocity.y -= (GRAVITY_CONST*gst->dt) * part->accel.y;


    part->color = ColorLerp(part->start_color, part->end_color, part->n_lifetime*2.0);

    *part->transform = MatrixMultiply(scale_matrix, translation);
}


// PARTICLE INITIALIZATION
void explosion_psys_init(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        Color part_color,
        void* extradata, int has_extradata
){
    if(!has_extradata) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Missing extradata pointer\033[0m\n",
                __func__);
        return;
    }

    // Explosion radius should be passed here.
    float exp_radius = *(float*)extradata;

    part->position = origin;
    const float p_r = 0.1;

    // Choose position.

    part->position.x += RSEEDRANDOMF(-p_r, p_r) + (8.0 * cos(part->index));
    part->position.y += RSEEDRANDOMF(-p_r, p_r);
    part->position.z += RSEEDRANDOMF(-p_r, p_r) + (8.0 * sin(part->index));
  

    // Choose particle behaviour

    if(part->idb == PART_IDB_EXPLOSION) {
        const float v_r = exp_radius * 0.5;
        part->velocity.x += RSEEDRANDOMF(-v_r,     v_r);
        part->velocity.y += RSEEDRANDOMF(-v_r*0.5, v_r * RSEEDRANDOMF(1.5, 3.0));
        part->velocity.z += RSEEDRANDOMF(-v_r,     v_r);
        part->accel.y = 0.2;
        part->scale = 8.0;
    
        part->max_lifetime = RSEEDRANDOMF(0.6, 1.25);
        part->start_color = (Color){ 255, 180, 40, 255 };
        part->end_color   = (Color){ 255, 30, 5, 255 };
    }
    else
    if(part->idb == PART_IDB_SMOKE) {
        const float v_r = 3.0;
        part->velocity.x += RSEEDRANDOMF(-v_r,     v_r);
        part->velocity.y += RSEEDRANDOMF( v_r*1.5, v_r*8.0);
        part->velocity.z += RSEEDRANDOMF(-v_r,     v_r);
        part->scale = RSEEDRANDOMF(8.0, 16.0); 
        part->accel.y = 0.0;
        part->scale = RSEEDRANDOMF(8.0, 16.0);
        
        part->max_lifetime = RSEEDRANDOMF(2.0, 5.0);
        part->start_color = (Color){ 50, 50, 50, 255 };
        part->end_color   = (Color){ 5, 5, 5, 255 };
    }
}


