#include "prj_envhit_psys.h"
#include "../state.h"
#include "../util.h"

#include <raymath.h>
#include <stdio.h>

#define DURATION 0.5

// PARTICLE UPDATE
void prj_envhit_psys_update(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
){

    const float scale_duration = part->max_lifetime;
    if(part->scale < scale_duration) {
        part->scale += gst->dt;
    }

    float st = lerp(part->scale / scale_duration, 0.0, 12.0);

    Matrix scale_matrix = MatrixScale(st, st, st);
    Matrix translation = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    
    *part->transform = MatrixMultiply(scale_matrix, translation);


    if(part->last_update) {

        struct psystem_t* nextpsys = NULL;
        if(psys->groupid == PSYS_GROUPID_PLAYER) {
            nextpsys = &gst->psystems[PLAYER_PRJ_ENVHIT_PART2_PSYS];
        }
        else
        if(psys->groupid == PSYS_GROUPID_ENEMY) {
            nextpsys = &gst->psystems[ENEMY_PRJ_ENVHIT_PART2_PSYS];
        }
        else {
            return;
        }

        add_particles(
                gst,
                nextpsys, 
                GetRandomValue(5, 10),
                part->position, (Vector3){0},
                NULL, NO_EXTRADATA, NO_IDB);
    }
}



// PARTICLE INITIALIZATION
void prj_envhit_psys_init(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        void* extradata, int has_extradata
){

    part->scale = 0.0;
    part->position = origin;
    
    *part->transform = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    part->max_lifetime = 0.2;
}


