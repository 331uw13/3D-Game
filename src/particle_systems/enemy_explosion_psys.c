#include "enemy_explosion_psys.h"
#include "../state.h"
#include "../util.h"

#include <raymath.h>
#include <stdio.h>


// PARTICLE UPDATE
void enemy_explosion_psys_update(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
){
    // TODO: This can be optimized.

    RayCollision ray = raycast_terrain(&gst->terrain, part->position.x, part->position.z);
    Matrix translation = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    
    float t = lerp(normalize(part->lifetime, 0.0, part->max_lifetime), 3.0, 0.0);
    Matrix scale_matrix = MatrixScale(t, t, t);


    part->position = Vector3Add(part->position, Vector3Scale(part->velocity, gst->dt));
    part->velocity.y -= (500.0*gst->dt) * part->accel.y;

    *part->transform = MatrixMultiply(scale_matrix, translation);
}



// PARTICLE INITIALIZATION
void enemy_explosion_psys_init(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        void* extradata, int has_extradata
){

    part->position = origin;
    const float p_r = 0.1;

    float x_r = 10.0 * cos(part->index);
    float z_r = 10.0 * sin(part->index);

    part->position.x += RSEEDRANDOMF(-p_r, p_r) + x_r;
    part->position.y += RSEEDRANDOMF(-p_r, p_r);
    part->position.z += RSEEDRANDOMF(-p_r, p_r) + z_r;
    Matrix transform = MatrixTranslate(part->position.x, part->position.y, part->position.z);
   
    part->velocity = Vector3Normalize(Vector3Negate(velocity));
    
    const float v_r = 20.0;
    part->velocity.x += RSEEDRANDOMF(-v_r, v_r);
    part->velocity.y += RSEEDRANDOMF(v_r, v_r*6.0);
    part->velocity.z += RSEEDRANDOMF(-v_r, v_r);


    part->accel.y = 0.1;

    *part->transform = transform;
    part->max_lifetime = RSEEDRANDOMF(1.0, 3.0);
}


