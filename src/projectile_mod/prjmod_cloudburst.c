#include <stdio.h>

#include "prjmod_cloudburst.h"
#include "../particle_systems/weapon_psys.h"
#include "state.h"
#include "util.h"


static void spawn_cloudburst(struct state_t* gst, struct psystem_t* psys, struct particle_t* part, Vector3 position) {
    float* timer = &psys->user_f[PWRUP_PRJ_CLOUDBURST_TIMER_I];
    if(*timer < 4.0) {
        return;
    }

    *timer = 0.0;
    position.y += 600.0;

    for(int i = 0; i < 8; i++) {
        add_projectile(gst, &gst->psystems[PLAYER_WEAPON_PSYS], &gst->player.weapon,
                position, (Vector3){ 0, RSEEDRANDOMF(-1.5, -2.25), 0 }, 2.0);
    }

    SetSoundPitch(gst->sounds[CLOUDBURST_SOUND], 1.0 - RSEEDRANDOMF(0.0, 0.025));
    PlaySound(gst->sounds[CLOUDBURST_SOUND]);

}


void prjmod_cloudburst__init(struct state_t* gst, struct psystem_t* psys, struct particle_t* part) {

}

void prjmod_cloudburst__update(struct state_t* gst, struct psystem_t* psys, struct particle_t* part) {
    
    // Only allow particles that have been alive short amount of time to contribute to the timer value
    // This is because if the player fires projectile somewhere it cant hit anything the timer will keep running
    if(part->lifetime < 2.0) {
        float* timer = &psys->user_f[PWRUP_PRJ_CLOUDBURST_TIMER_I];
        *timer += gst->dt / psys->num_alive_parts;
    }
    
}

void prjmod_cloudburst__env_hit(struct state_t* gst, struct psystem_t* psys, struct particle_t* part, Vector3 normal) {
    spawn_cloudburst(gst, psys, part, part->position);
}

int prjmod_cloudburst__enemy_hit(struct state_t* gst, struct psystem_t* psys, struct particle_t* part,
        struct enemy_t* ent, struct hitbox_t* hitbox, int* cancel_defdamage) {
    spawn_cloudburst(gst, psys, part, ent->position);
    return 1;
}
