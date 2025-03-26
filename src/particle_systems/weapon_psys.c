#include "weapon_psys.h"
#include <raymath.h>
#include <stdio.h>
#include "../state.h"
#include "../util.h"
#include "../enemy.h"



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
    struct weapon_t* weapon = (struct weapon_t*)part->extradata;
    if(!weapon) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Missing extradata pointer\033[0m\n",
                __func__);
        return;
    }

    if(weapon->gid >= INVLID_WEAPON_GID) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Invalid weapon id\033[0m\n",
                __func__);
        return;
    }


    /*
    if(weapon->gid == PLAYER_WEAPON_ID) {
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
    */

    Vector3 vel = Vector3Scale(part->velocity, gst->dt * weapon->prj_speed);
    part->position = Vector3Add(part->position, vel);

    Matrix transform = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    *part->transform = transform;

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
                (Color){0},
                NULL, NO_EXTRADATA, NO_IDB
                );
        disable_projectile(gst, part);
        return;
    }



    int env_hit = 0;

    // Check collision with terrain

    RayCollision t_hit = raycast_terrain(&gst->terrain, part->position.x, part->position.z);

    if(t_hit.point.y >= part->position.y) {

        env_hit = 1;
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
                env_hit = 1;
            }
        }
    }
    else
    if(psys->groupid == PSYS_GROUPID_ENEMY) {
        // Check collision with player.

        if(CheckCollisionBoxes(part_boundingbox, get_player_boundingbox(&gst->player))) {
            player_damage(gst, &gst->player, get_weapon_damage(weapon));
            env_hit = 1;
        }
    }

    if(env_hit) {
        add_particles(gst,
                &gst->psystems[PROJECTILE_ENVHIT_PSYS],
                1,
                part->position,
                (Vector3){0, 0, 0},
                part->color,
                NULL, NO_EXTRADATA, NO_IDB);
        disable_projectile(gst, part);
    }

}

void weapon_psys_prj_init(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        Color part_color,
        void* extradata, int has_extradata
){

    struct weapon_t* weapon = (struct weapon_t*)extradata;
    if(!weapon || !has_extradata) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Missing extradata pointer\033[0m\n",
                __func__);
        return;
    }

    part->extradata = extradata;
    part->velocity = velocity;
    part->position = origin;
    part->user_i[0] = 0;


    // Add projectile light
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

    part->color = weapon->color;
    part->has_light = 1;
    part->max_lifetime = weapon->prj_max_lifetime;

    add_particles(gst, 
            &gst->psystems[PRJ_TRAIL_PSYS], 
            6,
            (Vector3){0}, (Vector3){0}, (Color){0},
            part, HAS_EXTRADATA, NO_IDB);

}
