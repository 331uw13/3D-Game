#ifndef WEAPON_H
#define WEAPON_H

#include <stddef.h>
#include <raylib.h>



#define WEAPON_MAX_PROJECTILES 256


struct projectile_t {
    
    Vector3 position;
    Vector3 direction;
    Vector3 hitbox;

    float lifetime;
    int alive;

    unsigned int light_index;
};

struct weapon_t {
    
    //Model model;
    //Vector3 model_poffset; // model position offset

    struct projectile_t projectiles[WEAPON_MAX_PROJECTILES];
    size_t num_alive_projectiles;
    size_t proj_nextindex;


    float knockback;
    float proj_speed;
    float proj_damage;
    Vector3 proj_hitbox;
    float proj_max_lifetime;

    Shader  projectile_shader;

};


void setup_weapon(
        struct weapon_t* w, 
        float proj_speed,
        float proj_damage,
        float proj_knockback,
        float proj_max_lifetime,
        Vector3 proj_hitbox
        );



#endif
