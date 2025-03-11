#include "player_hit_psys.h"
#include "../state.h"
#include "../util.h"

#include <raymath.h>
#include <stdio.h>


// PARTICLE UPDATE
void player_hit_psys_update(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
){
    // TODO: This can be optimized.

    RayCollision ray = raycast_terrain(&gst->terrain, part->position.x, part->position.z);
    Matrix scale_matrix = MatrixIdentity();

    if(part->position.y >= ray.point.y) {
        part->velocity.y -= part->accel.y * gst->dt;
        part->position = Vector3Add(part->position, Vector3Scale(part->velocity, gst->dt*40));
    }
    else {
        scale_matrix = MatrixScale(2.0, 0.5, 2.0);
    }

    Matrix translation = MatrixTranslate(part->position.x, part->position.y, part->position.z);


    *part->transform = MatrixMultiply(scale_matrix, translation);
}



// PARTICLE INITIALIZATION
void player_hit_psys_init(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        void* extradata, int has_extradata
){

    part->position = origin;
    const float p_r = 0.1;
    part->position.x += RSEEDRANDOMF(-p_r, p_r);
    part->position.y += RSEEDRANDOMF(-p_r, p_r);
    part->position.z += RSEEDRANDOMF(-p_r, p_r);
    Matrix transform = MatrixTranslate(part->position.x, part->position.y, part->position.z);
   
    part->velocity = Vector3Normalize(Vector3Negate(velocity));
    
    const float v_r = 0.5;
    part->velocity.x += RSEEDRANDOMF(-v_r, v_r);
    part->velocity.y += RSEEDRANDOMF(-v_r*0.5, v_r*2);
    part->velocity.z += RSEEDRANDOMF(-v_r, v_r);
   
    part->accel.y = 5.0;

    *part->transform = transform;
    part->max_lifetime = RSEEDRANDOMF(5.0, 8.0);
}


