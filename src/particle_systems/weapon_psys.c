#include "weapon_psys.h"
#include <raymath.h>
#include <stdio.h>
#include "../state.h"
#include "../util.h"


#define MISSING_PSYSUSERPTR fprintf(stderr, "\033[31m(ERROR) '%s': Missing psystem 'userptr'\033[0m\n", __func__)


void weapon_psys_prj_update(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
){
    struct weapon_t* weapon = (struct weapon_t*)psys->userptr;
    if(!weapon) {
        MISSING_PSYSUSERPTR;
        return;
    }

    if(weapon->id >= INVLID_WEAPON_ID) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Invalid weapon id\033[0m\n",
                __func__);
        return;
    }

    Vector3 vel = Vector3Scale(part->velocity, gst->dt * weapon->prj_speed);
    part->position = Vector3Add(part->position, vel);

    Matrix position_m = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    *part->transform = position_m;

    part->light.position = part->position;
    update_light_values(&part->light, gst->shaders[DEFAULT_SHADER]);


    // Check collision with terrain

    RayCollision t_hit = raycast_terrain(&gst->terrain, part->position.x, part->position.z);

    if(t_hit.point.y >= part->position.y) {
        disable_particle(gst, part);

        struct psystem_t* psystem = NULL;
        if(weapon->id == PLAYER_WEAPON_ID) {
            psystem = &gst->psystems[PLAYER_PRJ_ENVHIT_PSYS];
        }
        else
        if(weapon->id == ENEMY_WEAPON_ID) {
            psystem = &gst->psystems[ENEMY_PRJ_ENVHIT_PSYS];
        }

        add_particles(
                gst,
                psystem,
                1,
                part->position,
                (Vector3){0, 0, 0},
                NULL,
                NO_EXTRADATA
                );

        return;
    }
}

void weapon_psys_prj_init(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        void* extradata, int has_extradata
){
    struct weapon_t* weapon = (struct weapon_t*)psys->userptr;
    if(!weapon) {
        MISSING_PSYSUSERPTR;
        return;
    }

    part->velocity = velocity;
    part->position = origin;
    Matrix position_m = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    Matrix rotation_m = MatrixRotateXYZ((Vector3){
            RSEEDRANDOMF(-3.0, 3.0), 0.0, RSEEDRANDOMF(-3.0, 3.0)
            });

    position_m = MatrixMultiply(position_m, rotation_m);
    add_projectile_light(gst, &part->light, part->position, weapon->color, gst->shaders[DEFAULT_SHADER]);
    part->has_light = 1;

    *part->transform = position_m;
    part->max_lifetime = weapon->prj_max_lifetime;
}
