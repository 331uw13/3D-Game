#include <stdio.h>
#include <math.h>

#include "prjmod_fmj_ability.h"
#include "../state/state.h"
#include "../particle_systems/weapon_psys.h"



void prjmod_fmj__init(struct state_t* gst, struct psystem_t* psys, struct particle_t* part) {
    printf("'%s'\n", __func__);


}

void prjmod_fmj__update(struct state_t* gst, struct psystem_t* psys, struct particle_t* part) {
}



void prjmod_fmj__env_hit(struct state_t* gst, struct psystem_t* psys, struct particle_t* part, Vector3 normal) {
    printf("'%s'\n", __func__);


}

int prjmod_fmj__enemy_hit(struct state_t* gst, struct psystem_t* psys, struct particle_t* part,
        struct enemy_t* ent, struct hitbox_t* hitbox, int* cancel_defdamage) {
    *cancel_defdamage = 0;
    int disable_prj = 1;
    int powerup_level = round(gst->player.powerup_levels[POWERUP_FMJPRJ_ABILITY]);
    if(powerup_level <= 0) {
        fprintf(stderr, "\033[35m(WARNING) '%s': FMJ Projectile ability level is zero or less"
                " but this function was called.\033[0m\n",
                __func__);
        goto skip;
    }

    if(part->user_i[PWRUP_ENT_PASSED_I] < powerup_level) {
        // Skip the current hitbox that was hit.
        part->position.x += part->velocity.x * (hitbox->size.x*1.5);
        part->position.y += part->velocity.y * (hitbox->size.y*1.5);
        part->position.z += part->velocity.z * (hitbox->size.z*1.5);
        part->user_i[PWRUP_ENT_PASSED_I]++;
        disable_prj = 0;
    }
skip:
    return disable_prj;
}


