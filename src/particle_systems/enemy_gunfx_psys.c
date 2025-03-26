#include "enemy_gunfx_psys.h"
#include "../state.h"
#include "../util.h"
#include "../enemy.h"

#include <raymath.h>
#include <stdio.h>


// PARTICLE UPDATE
void enemy_gunfx_psys_update(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
){
    if(!part->extradata) {
        return;
    }

    struct enemy_t* ent = (struct enemy_t*)part->extradata;
    if(!ent) {
        return;
    }


    Matrix rotation_matrix = MatrixTranslate(0, 0, 0);
    rotation_matrix = MatrixMultiply(MatrixRotateY(ent->rotation.y), rotation_matrix);
    rotation_matrix = MatrixMultiply(MatrixRotateZ(ent->rotation.z+1.570795), rotation_matrix);



    const float duration = 0.2;
    if(part->scale < duration) {
        part->scale += gst->dt;
    }

    float st = lerp(part->scale / duration, 15.0, 0.0);
    Matrix scale_matrix = MatrixScale(st, st, st);
    Matrix translation = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    
    *part->transform = MatrixMultiply(MatrixMultiply(scale_matrix, rotation_matrix), translation);
}



// PARTICLE INITIALIZATION
void enemy_gunfx_psys_init(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        Color part_color,
        void* extradata, int has_extradata
){
    if(!extradata || !has_extradata) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Missing extradata pointer\033[0m\n",
                __func__);
        return;
    }

    part->color = part_color;
    part->extradata = extradata;
    part->position = origin;
    part->scale = 0.0; // scale lerp.
    part->max_lifetime = 1.0;
}


