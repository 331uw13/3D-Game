#include "weapon.h"


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


    for(size_t i = 0; i < WEAPON_MAX_PROJECTILES; i++) {
        struct projectile_t* p = &w->projectiles[i];
        p->position = (Vector3){ 0.0, 0.0, 0.0 };
        p->direction = (Vector3){ 0.0, 0.0, 0.0 };
        p->hitbox = (Vector3){ 0.0, 0.0, 0.0 };
        p->lifetime = 0.0;
        p->alive = 0;
    }



}
