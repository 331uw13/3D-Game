#include "prj_trail_psys.h"
#include "../state/state.h"
#include "../util.h"


#include <raymath.h>
#include <stdio.h>


// PARTICLE UPDATE
void prj_trail_psys_update(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
){
    struct particle_t* tofollow_part = (struct particle_t*)part->extradata;

    if(((part->lifetime + gst->dt) >= part->max_lifetime) && tofollow_part->alive) {
        prj_trail_psys_init(gst, psys, part, (Vector3){0}, (Vector3){0}, (Color){0}, tofollow_part, HAS_EXTRADATA);
        part->lifetime = 0.0;
        return;
    }

    float scalelrp = lerp(part->n_lifetime, tofollow_part->scale, 0.0);
    Matrix scale_matrix = MatrixScale(scalelrp, scalelrp, scalelrp);

    part->color = ColorLerp(part->start_color, part->end_color, part->n_lifetime*2.0);//tofollow_part->color;

    Matrix translation = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    *part->transform = MatrixMultiply(scale_matrix, translation);
}



// PARTICLE INITIALIZATION
void prj_trail_psys_init(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        Color part_color,
        void* extradata, int has_extradata
){
    struct particle_t* tofollow_part = (struct particle_t*)extradata;

    part->start_color = tofollow_part->color;
    part->end_color = (Color) {
        tofollow_part->color.r,
        tofollow_part->color.g,
        tofollow_part->color.b,
        0
    };

    part->position = tofollow_part->position;
    part->position.x += tofollow_part->velocity.x*6;
    part->position.y += tofollow_part->velocity.y*6;
    part->position.z += tofollow_part->velocity.z*6;
    part->extradata = extradata;
    part->max_lifetime = RSEEDRANDOMF(0.001, 0.1);
    part->lifetime = part->max_lifetime;
}


