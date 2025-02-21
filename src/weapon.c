#include <stdio.h>

#include "state.h"
#include "weapon.h"
#include "util.h"

/*
void setup_weapon(
        struct weapon_t* w, 
        float proj_speed,
        float proj_damage,
        float proj_knockback,
        float proj_max_lifetime,
        Vector3 proj_hitbox
        )
{

    w->proj_speed = proj_speed;
    w->proj_damage = proj_damage;
    w->proj_hitbox = proj_hitbox;
    w->num_alive_projectiles = 0;
    w->proj_nextindex = 0;
    w->proj_max_lifetime = proj_max_lifetime;
    w->knockback = proj_knockback;


    for(size_t i = 0; i < MAX_WEAPON_PROJECTILES; i++) {
        struct projectile_t* p = &w->projectiles[i];
        p->position = (Vector3){ 0.0, 0.0, 0.0 };
        p->direction = (Vector3){ 0.0, 0.0, 0.0 };
        p->hitbox = (Vector3){ 0.0, 0.0, 0.0 };
        p->lifetime = 0.0;
        p->alive = 0;
    }

}
*/

static struct projectile_t* get_nextprojectile(struct weapon_t* w) {
    return (w->prj_nextindex < MAX_WEAPON_PROJECTILES) 
        ? (&w->projectiles[w->prj_nextindex]) : NULL;
}

float compute_weapon_accuracy(struct state_t* gst, struct weapon_t* weapon) {
    float k = 0.1;
    return k-map(weapon->accuracy, 0, 10, 0.0, k);
}


void weapon_add_projectile(
        struct state_t* gst,
        struct weapon_t* w,
        Vector3 position,
        Vector3 direction
){

    struct projectile_t* proj = get_nextprojectile(w);
    if(!proj) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Failed to get next projectile. invalid index?\033[0m\n",
                __func__);
        return;
    }

    if(proj->alive) {
        printf("\033[35m(WARNING) '%s': Next projectile is still alive. Overwriting data\033[0m\n",
                __func__);
    }

    proj->position = position;
    proj->direction = direction;
    proj->alive = 1;
    proj->lifetime = 0.0;

    float ac = compute_weapon_accuracy(gst, w);

    proj->direction.y += RSEEDRANDOMF(-ac, ac);
    proj->direction.x += RSEEDRANDOMF(-ac, ac);
    proj->direction.z += RSEEDRANDOMF(-ac, ac);


    add_light(gst,
            &proj->light,
            LIGHT_POINT,
            (Vector3){ 0 }, // position is updated later.
            (Color){ 10, 255, 255, 255 },
            gst->shaders[DEFAULT_SHADER]
            );
    

    w->num_alive_projectiles++;
    
    w->prj_nextindex++;
    if(w->prj_nextindex >= MAX_WEAPON_PROJECTILES) {
        w->prj_nextindex = 0;
    }
}


void weapon_update_projectiles(
        struct state_t* gst,
        struct weapon_t* w
){

    struct projectile_t* proj = NULL;

    // Keep track of how many was updated.
    // If this is equal to w->num_alive_projectiles exit loop
    size_t num_updated = 0; 

    BeginShaderMode(gst->shaders[PLAYER_PROJECTILE_SHADER]);

    for(size_t i = 0; i < MAX_WEAPON_PROJECTILES; i++) {
        proj = &w->projectiles[i];
        if(!proj->alive) {
            continue;
        }

        proj->lifetime += gst->dt;
        if(proj->lifetime >= w->prj_max_lifetime) {
            proj->alive = 0;
            disable_light(&proj->light, gst->shaders[DEFAULT_SHADER]);
            continue;
        }


        // Scale direction with projectile speed.
        proj->position.x += (proj->direction.x * w->prj_speed) * gst->dt;
        proj->position.y += (proj->direction.y * w->prj_speed) * gst->dt;
        proj->position.z += (proj->direction.z * w->prj_speed) * gst->dt;


        // Render the projectile.
        
        // inner sphere
        DrawSphere(proj->position, 0.1, (Color){ 200, 255, 255, 255 });
        
        // outer sphere
        DrawSphere(proj->position, 0.2, (Color){ 10, 255, 255, 200 });


        // Update light position.
        proj->light.position = proj->position;
        update_light_values(&proj->light, gst->shaders[DEFAULT_SHADER]);

        num_updated++;
        if(num_updated >= w->num_alive_projectiles) {
            break;
        }
    }
    EndShaderMode();


    w->num_alive_projectiles = num_updated;
}


