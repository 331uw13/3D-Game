#include "weapon_psys.h"
#include <raymath.h>
#include <stdio.h>
#include "../state.h"
#include "../util.h"
#include "../enemy.h"


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

    Matrix transform = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    *part->transform = transform;

    part->light.position = part->position;
    set_light(gst, &part->light, gst->prj_lights_ubo);


    // Check collision with water

    if(part->position.y <= gst->terrain.water_ylevel) {
        add_particles(
                gst,
                &gst->psystems[WATER_SPLASH_PSYS],
                GetRandomValue(10, 20),
                part->position,
                part->velocity,
                NULL, NO_EXTRADATA
                );       
        disable_particle(gst, part);
        return;
    }


    // Check collision with terrain

    RayCollision t_hit = raycast_terrain(&gst->terrain, part->position.x, part->position.z);

    if(t_hit.point.y >= part->position.y) {

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
                NULL, NO_EXTRADATA
                );

    
        /*
        if(gst->has_audio) {
            SetSoundPitch(gst->sounds[PRJ_ENVHIT_SOUND], 1.0 - RSEEDRANDOMF(0.0, 0.25));
            SetSoundVolume(gst->sounds[PRJ_ENVHIT_SOUND], get_volume_dist(gst->player.position, part->position));
            PlaySound(gst->sounds[PRJ_ENVHIT_SOUND]);
        }
        */

        disable_particle(gst, part);
        return;
    }


    BoundingBox part_boundingbox = (BoundingBox) {
        (Vector3) { // Min box corner
            part->position.x - weapon->prj_hitbox_size.x/2,
            part->position.y - weapon->prj_hitbox_size.y/2,
            part->position.z - weapon->prj_hitbox_size.z/2
        },
        (Vector3) { // Max box corner
            part->position.x + weapon->prj_hitbox_size.x/2,
            part->position.y + weapon->prj_hitbox_size.y/2,
            part->position.z + weapon->prj_hitbox_size.z/2
        }
    };

    // TODO: Optimize this! <---
   
    if(psys->groupid == PSYS_GROUPID_PLAYER) {
        // Check collision with enemies.
        
        struct enemy_t* enemy = NULL;
        for(size_t i = 0; i < MAX_ALL_ENEMIES; i++) {
            enemy = &gst->enemies[i];
            if(!enemy->alive) {
                continue;
            }

            struct hitbox_t* hit = check_collision_hitboxes(&part_boundingbox, enemy);
            if(hit) {
                enemy_hit(gst, enemy, weapon, hit->damage_mult, part->position, part->velocity);

                disable_particle(gst, part);

                add_particles(gst,
                        &gst->psystems[PLAYER_PRJ_ENVHIT_PSYS],
                        1,
                        part->position,
                        (Vector3){0, 0, 0},
                        NULL, NO_EXTRADATA
                        );

                add_particles(gst,
                        &gst->psystems[ENEMY_HIT_PSYS],
                        GetRandomValue(10, 30),
                        part->position,
                        part->velocity,
                        NULL, NO_EXTRADATA
                        );


            }
        }
    }
    else
    if(psys->groupid == PSYS_GROUPID_ENEMY) {
        // Check collision with player.

        if(CheckCollisionBoxes(part_boundingbox, get_player_boundingbox(&gst->player))) {
            //player_hit(gst, &gst->player, weapon);

            player_damage(gst, &gst->player, get_weapon_damage(weapon, NULL));

            add_particles(gst,
                    &gst->psystems[ENEMY_PRJ_ENVHIT_PSYS],
                    1,
                    part->position,
                    (Vector3){0, 0, 0},
                    NULL, NO_EXTRADATA
                    );

            /*
            add_particles(gst,
                    &gst->psystems[PLAYER_HIT_PSYS],
                    GetRandomValue(5, 10),
                    part->position,
                    part->velocity,
                    NULL, NO_EXTRADATA
                    );
                    */

            disable_particle(gst, part);
        }
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
    Matrix transform = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    Matrix rotation_m = MatrixRotateXYZ((Vector3){
            RSEEDRANDOMF(-3.0, 3.0), 0.0, RSEEDRANDOMF(-3.0, 3.0)
            });

    transform = MatrixMultiply(transform, rotation_m);

    
    part->light = (struct light_t) {
        .enabled = 1,
        .type = LIGHT_POINT,
        .color = weapon->color,
        .strength = 1.0,
        .index = gst->num_prj_lights
        // position is updated later.
    };


    gst->num_prj_lights++;
    if(gst->num_prj_lights >= MAX_PROJECTILE_LIGHTS) {
        gst->num_prj_lights = 0;
    }

    part->has_light = 1;
    *part->transform = transform;
    part->max_lifetime = weapon->prj_max_lifetime;
}
