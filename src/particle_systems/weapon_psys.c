#include <stdio.h>

#include "weapon_psys.h"


#include "../state/state.h"
#include "../util.h"
#include "../enemy.h"

#include <raymath.h>


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
    if((gst->player.powerup_levels[POWERUP_GRAVITY_PROJECTILES] > 0.0)
    && (weapon->gid == PLAYER_WEAPON_GID)) {

    }
    */
   
    //Vector3 part_old_position = part->position;

    Vector3 vel = Vector3Scale(part->velocity, gst->dt * weapon->prj_speed);
    part->position = Vector3Add(part->position, vel);

    Matrix scale_matrix = MatrixScale(weapon->prj_scale, weapon->prj_scale, weapon->prj_scale);
    Matrix translation = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    *part->transform = MatrixMultiply(scale_matrix, translation);

    part->light.color = part->color;
    part->light.position = part->position;
    set_light(gst, &part->light, PRJLIGHTS_UBO);


    // Check collision with water
    /*
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
    */

    int disable_prj = 0;

    // Check collision with terrain
    RayCollision t_hit = raycast_terrain(&gst->terrain, part->position.x, part->position.z);
    if(t_hit.point.y >= part->position.y) {
        disable_prj = 1;
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
  

    if(weapon->gid == PLAYER_WEAPON_GID) {
        // Check collision with enemies.

        // TODO: Chunks should take care of enemies so this can be optimized alot.
        
        struct enemy_t* enemy = NULL;
        
        for(size_t i = 0; i < MAX_ALL_ENEMIES; i++) {
            enemy = &gst->enemies[i];
            if(!enemy->alive) {
                continue;

            }

            // Check collision radius.
            if(Vector3Distance(part->position, enemy->position) > enemy->ccheck_radius) {
                continue;
            }

            for(int i = 0; i < ENEMY_MAX_HITBOXES; i++) {
                struct hitbox_t* hitbox = &enemy->hitboxes[i];
                
                if(raycast_hitbox(
                            enemy->position,
                            hitbox,
                            part->prev_position,
                            part->position,
                            2.0
                            )) {
                    // Hit.

                    float damage = get_weapon_damage(weapon);
                    enemy_damage(gst, enemy, damage, hitbox, part->position, part->velocity, weapon->knockback);
                    disable_prj = 1;
                    goto collision_finished;
                }
            }
        }
collision_finished:
    }

    /*
    if(psys->groupid == PSYS_GROUPID_PLAYER) {
        // Check collision with enemies.
        
        // The raycasting can be only done when it is very nearby the enemy.
        // or it will waste alot of time that doesnt do anything useful.


        struct enemy_t* enemy = NULL;
        for(size_t i = 0; i < MAX_ALL_ENEMIES; i++) {
            enemy = &gst->enemies[i];
            if(!enemy->alive) {
                continue;
            }

            struct hitbox_t* hitbox = check_collision_hitboxes(&part_boundingbox, enemy);
            if(hitbox) {
                float damage = get_weapon_damage(weapon);
                float knockback = 0.35;
                enemy_damage(gst, enemy, damage, hitbox, part->position, part->velocity, knockback);
                disable_prj = 1;
            }
        }
    }
    else
    if(psys->groupid == PSYS_GROUPID_ENEMY) {
        // Check collision with player.

        if(CheckCollisionBoxes(part_boundingbox, get_player_boundingbox(&gst->player))) {
            player_damage(gst, &gst->player, get_weapon_damage(weapon));
            disable_prj = 1;
        }
    }
    */

    if(disable_prj) {
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

    // Add projectile light
    part->light = (struct light_t) {
        .enabled = 1,
        .type = LIGHT_POINT,
        .color = weapon->color,
        .strength = 1.25,
        .radius = 10.0,
        .index = gst->num_prj_lights
        // position is updated later.
    };


    part->scale = weapon->prj_scale;
    part->color = weapon->color;
    part->has_light = 1;
    part->max_lifetime = weapon->prj_max_lifetime;

    // Projectile trail.
    add_particles(gst, 
            &gst->psystems[PRJ_TRAIL_PSYS], 
            20,
            (Vector3){0}, (Vector3){0}, (Color){0},
            part, HAS_EXTRADATA, NO_IDB);


    gst->num_prj_lights++;
    if(gst->num_prj_lights >= MAX_PROJECTILE_LIGHTS) {
        gst->num_prj_lights = 0;
    }

}
