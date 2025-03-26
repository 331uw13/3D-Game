#include "weapon_psys.h"
#include <raymath.h>
#include <stdio.h>
#include "../state.h"
#include "../util.h"
#include "../enemy.h"


#define MISSING_PSYSUSERPTR fprintf(stderr, "\033[31m(ERROR) '%s': Missing psystem 'userptr'\033[0m\n", __func__)


static void disable_projectile(struct state_t* gst, struct particle_t* part) {
    disable_particle(gst, part);
    part->light.position.y += 5.0;
    add_decay_light(gst, &part->light, 14.0);
}


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


    if(weapon->id == PLAYER_WEAPON_ID) {
        if(!part->user_i[0]) {
            
            float closest = 99999999;
            Vector3 point;

            for(size_t i = 0; i < gst->num_enemies; i++) {
                float pdiste = Vector3Distance(gst->enemies[i].position, part->position);
                if(!gst->enemies[i].alive) {
                    continue;
                }

                if(closest > pdiste) {
                    closest = pdiste;
                    point = gst->enemies[i].position;
                    part->user_i[0] = 1;
                    part->user_v[0] = gst->enemies[i].position;
                }
            }
        }

        if(part->user_i[0]) {
            float ent_mass = 20.0;
            float part_mass = 0.5;

            float dist = Vector3Distance(part->user_v[0], part->position);
            Vector3 dir = Vector3Subtract(part->user_v[0], part->position);
            float mag = ((ent_mass * part_mass)*0.2) / (dist * dist);

            dir = Vector3Scale(dir, mag);

            part->velocity = Vector3Add(part->velocity, dir);
        }
    }


    Vector3 vel = Vector3Scale(part->velocity, gst->dt * weapon->prj_speed);
    part->position = Vector3Add(part->position, vel);

    Matrix transform = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    *part->transform = transform;

    // FOR TESTING.
    part->color.a = 255;
    rainbow_palette(sin(part->lifetime), &part->color.r, &part->color.g, &part->color.b);



    part->light.color = part->color;
    part->light.position = part->position;
    set_light(gst, &part->light, PRJLIGHTS_UBO);

   
    // Check collision with water

    if(part->position.y <= gst->terrain.water_ylevel) {
        add_particles(
                gst,
                &gst->psystems[WATER_SPLASH_PSYS],
                GetRandomValue(10, 20),
                part->position,
                part->velocity,
                NULL, NO_EXTRADATA, NO_IDB
                );
        disable_projectile(gst, part);
        //disable_particle(gst, part);
        return;
    }




    // Check collision with terrain

    RayCollision t_hit = raycast_terrain(&gst->terrain, part->position.x, part->position.z);

    if(t_hit.point.y >= part->position.y) {

        //create_explosion(gst, part->position, 100.0, 100.0);

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
                NULL, NO_EXTRADATA, NO_IDB
                );

    
        /*
        if(gst->has_audio) {
            SetSoundPitch(gst->sounds[PRJ_ENVHIT_SOUND], 1.0 - RSEEDRANDOMF(0.0, 0.25));
            SetSoundVolume(gst->sounds[PRJ_ENVHIT_SOUND], get_volume_dist(gst->player.position, part->position));
            PlaySound(gst->sounds[PRJ_ENVHIT_SOUND]);
        }
        */

        disable_projectile(gst, part);
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

            struct hitbox_t* hitbox = check_collision_hitboxes(&part_boundingbox, enemy);
            if(hitbox) {
                float damage = get_weapon_damage(weapon);
                enemy_damage(gst, enemy, damage, hitbox, part->position, part->velocity, 0.35);
        
                disable_projectile(gst, part);

                add_particles(gst,
                        &gst->psystems[PLAYER_PRJ_ENVHIT_PSYS],
                        1,
                        part->position,
                        (Vector3){0, 0, 0},
                        NULL, NO_EXTRADATA, NO_IDB
                        );

            }
        }
    }
    else
    if(psys->groupid == PSYS_GROUPID_ENEMY) {
        // Check collision with player.

        if(CheckCollisionBoxes(part_boundingbox, get_player_boundingbox(&gst->player))) {
            //player_hit(gst, &gst->player, weapon);

            player_damage(gst, &gst->player, get_weapon_damage(weapon));

            add_particles(gst,
                    &gst->psystems[ENEMY_PRJ_ENVHIT_PSYS],
                    1,
                    part->position,
                    (Vector3){0, 0, 0},
                    NULL, NO_EXTRADATA, NO_IDB
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

            
            disable_projectile(gst, part);
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

    part->user_i[0] = 0;
    
    part->light = (struct light_t) {
        .enabled = 1,
        .type = LIGHT_POINT,
        .color = weapon->color,
        .strength = 1.0,
        .index = gst->num_prj_lights,
        .radius = 10.0
        // position is updated later.
    };


    gst->num_prj_lights++;
    if(gst->num_prj_lights >= MAX_PROJECTILE_LIGHTS) {
        gst->num_prj_lights = 0;
    }

    part->color = gst->player.weapon.color;

    part->has_light = 1;
    *part->transform = transform;
    part->max_lifetime = weapon->prj_max_lifetime;

    if(weapon->id == PLAYER_WEAPON_ID) {
        add_particles(gst, 
                &gst->psystems[PRJ_TRAIL_PSYS], 6, (Vector3){0}, (Vector3){0},
                part, HAS_EXTRADATA, NO_IDB);
    }

}
