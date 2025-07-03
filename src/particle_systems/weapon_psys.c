#include <stdio.h>

#include "weapon_psys.h"
#include "projectile_envhit_psys.h"

#include "../state/state.h"
#include "../util.h"
#include "../enemy.h"

#include <raymath.h>


/*
static void disable_projectile(struct state_t* gst, struct particle_t* part) {
    disable_particle(gst, part);
    
    //part->light.position.y += 5.0;
    //add_decay_light(gst, &part->light, 14.0);
}
*/

// For future improvements.
#define HITOBJ_TERRAIN 0
#define HITOBJ_ENEMY 1
#define HITOBJ_PLAYER 2

// This function handles what happens to the projectile after it hit something.
// If 'hit_object' is HITOBJ_TERRAIN, normal is set. otherwise its zero.
static void post_projectile_hit(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part,
        int hit_object,
        Vector3 normal
){

    
    if(hit_object == HITOBJ_TERRAIN) {
        part->position = 
            Vector3Add(part->position, Vector3Scale(normal, 4.0));
    }


    Vector3 nextpart_velocity = part->velocity;
    if(hit_object == HITOBJ_TERRAIN) {
        nextpart_velocity = Vector3Reflect(part->velocity, normal);
    }
    else {
        nextpart_velocity = Vector3Scale(nextpart_velocity, 0.75);
    }

    add_particles(gst,
            &gst->psystems[PROJECTILE_ENVHIT_PSYS],
            40,
            part->position,
            nextpart_velocity,
            part->color,
            NULL, NO_EXTRADATA, NO_IDB);  

    disable_particle(gst, part);
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

    if(weapon->gid >= INVALID_WEAPON_GID) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Invalid weapon id\033[0m\n",
                __func__);
        return;
    }


    Vector3 vel = Vector3Scale(part->velocity, gst->dt * weapon->prj_speed);
    part->position = Vector3Add(part->position, vel);

    Matrix scale_matrix = MatrixScale(weapon->prj_scale, weapon->prj_scale, weapon->prj_scale);
    Matrix translation = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    *part->transform = MatrixMultiply(scale_matrix, translation);

    if(part->has_light && part->light) {
        part->light->position = part->position;
    }
    

    // Check collision with terrain
    RayCollision t_hit = raycast_terrain(&gst->terrain, part->position.x, part->position.z);
    if(t_hit.point.y >= part->position.y) {
        post_projectile_hit(gst, psys, part, HITOBJ_TERRAIN, t_hit.normal);
        return;
    }


    const float weapon_damage = get_weapon_damage(weapon);

    if(weapon->gid == PLAYER_WEAPON_GID) {
        // Check collision with enemies.
        
        struct enemy_t* enemy = NULL;
        struct chunk_t* p_chunk = find_chunk(gst, part->position);

        for(uint16_t i = 0; i < p_chunk->num_enemies; i++) {
            enemy = &p_chunk->enemies[i];


            // Check collision radius.
            if(Vector3Distance(part->position, enemy->position) > enemy->ccheck_radius) {
                continue;
            }

            // Which hitbox was hit.
            for(size_t i = 0; i < enemy->num_hitboxes; i++) {
                struct hitbox_t* hitbox = &enemy->hitboxes[i];
                
                if(raycast_hitbox(
                            enemy->position,
                            hitbox,
                            part->prev_position,
                            part->position,
                            weapon->prj_scale
                            )) {
                    // Hit.
                    enemy_damage(gst,
                            enemy,
                            weapon_damage,
                            hitbox,
                            part->position,
                            part->velocity,
                            weapon->knockback);

                    post_projectile_hit(gst, psys, part, HITOBJ_ENEMY, (Vector3){0});
                    break;
                }
            }
        }
    }
    else
    if(weapon->gid == ENEMY_WEAPON_GID) { // Check collision with player.

        // Check collision radius.
        if(Vector3Distance(part->position, gst->player.position)
        < gst->player.ccheck_radius) {
            
            // Which hitbox was hit.
            for(size_t i = 0; i < MAX_HITBOXES; i++) {
                struct hitbox_t* hitbox = &gst->player.hitboxes[i];

                if(raycast_hitbox(
                            gst->player.position,
                            hitbox,
                            part->prev_position,
                            part->position,
                            weapon->prj_scale
                            )) {
                    // Hit.
                    player_damage(gst, &gst->player, weapon_damage);
                    post_projectile_hit(gst, psys, part, HITOBJ_PLAYER, (Vector3){0});
                    break;
                }
            }
        }
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
        fprintf(stderr, "\033[31m(ERROR) '%s': Missing extradata pointer (weapon)\033[0m\n",
                __func__);
        return;
    }

    part->extradata = extradata;
    part->velocity = velocity;
    part->position = origin;

    part->scale = weapon->prj_scale;
    part->color = weapon->color;
    part->max_lifetime = weapon->prj_max_lifetime;


    part->light = add_light(gst, (struct light_t) {
        .color = weapon->color,
        .radius = 16.0,
        .strength = 1.0,
        .position = part->position,
        .preserve = 0
    },
    NEVER_OVERWRITE);

    part->has_light = (part->light != NULL);

    // Projectile trail.
    add_particles(gst, 
            &gst->psystems[PRJ_TRAIL_PSYS], 
            20,
            (Vector3){0}, (Vector3){0}, (Color){0},
            part, HAS_EXTRADATA, NO_IDB);

}

