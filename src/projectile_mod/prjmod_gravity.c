#include <stdio.h>

#include "prjmod_test.h"
#include "../state.h"
#include "../particle_systems/weapon_psys.h"

#include <raymath.h>


void prjmod_gravity__init(struct state_t* gst, struct psystem_t* psys, struct particle_t* part) {

}

void prjmod_gravity__update(struct state_t* gst, struct psystem_t* psys, struct particle_t* part) {


    // Gravitate projectile towards enemies.

    short* prj_has_target = &part->user_i[PWRUP_PRJ_HAS_GRAVITY_TARGET_I];
    short* prj_ent_index = &part->user_i[PWRUP_PRJ_GRAVITY_TARGET_I];

    if(!*prj_has_target) {

        // Projectile doesnt have any target, try to find closest enemy.
        float closest = 9999999;
        for(size_t i = 0; i < gst->num_enemies; i++) {
            struct enemy_t* ent = &gst->enemies[i];
            if(!ent->alive) {
                continue;
            }
            float pdiste = Vector3Distance(part->position, ent->position);

            // TODO: Maybe range check for this?

            if(pdiste < closest) {
                closest = pdiste;
                *prj_has_target = 1;
                *prj_ent_index = i;
            }
        }

    }
    
    // If target was found gravitate towards it.
    if(*prj_has_target && (*prj_ent_index >= 0 && *prj_ent_index < MAX_ALL_ENEMIES)) {
        const float ent_mass = 20.0;
        const float prj_mass = 0.5;

        struct enemy_t* ent = &gst->enemies[*prj_ent_index];

        if(!ent->alive) {
            *prj_has_target = 0;
        }

        float pdiste = Vector3Distance(ent->position, part->position);
        Vector3 direction = Vector3Subtract(ent->position, part->position);
        float magnitude = ((ent_mass * prj_mass)*0.2) / (pdiste * pdiste);

        direction = Vector3Scale(direction, magnitude);
        part->velocity = Vector3Add(part->velocity, Vector3Scale(direction, gst->dt*500.0));

        part->velocity = Vector3Scale(part->velocity, pow(0.999, gst->dt*TARGET_FPS));
    }
}



void prjmod_gravity__env_hit(struct state_t* gst, struct psystem_t* psys, struct particle_t* part, Vector3 normal) {

}

int prjmod_gravity__enemy_hit(struct state_t* gst, struct psystem_t* psys, struct particle_t* part, 
        struct enemy_t* ent, struct hitbox_t* hitbox, int* cancel_defdamage) {
    
    return 0;
}



