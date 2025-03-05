#include <stdio.h>

#include "state.h"
#include "enemy.h"
#include "util.h"

#include "enemies/enemy_lvl0.h"



struct enemy_t* create_enemy(
        struct state_t* gst,
        int enemy_type,
        int texture_id,
        const char* model_filepath,
        struct psystem_t* weapon_psysptr,
        struct weapon_t* weaponptr,
        int max_health,
        Vector3 initial_position,
        Vector3 hitbox_size,
        Vector3 hitbox_position,
        float target_range,
        float firerate
){
    struct enemy_t* entptr = NULL;

    if((gst->num_enemies+1) >= MAX_ENEMIES) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Max enemies reached.\033[0m\n",
                __func__);
        goto error;
    }

    if(!FileExists(model_filepath)) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Model file path not found: '%s'\033[0m\n",
                __func__, model_filepath);
        goto error;
    }

    if(texture_id >= MAX_TEXTURES) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Invalid texture id: %i for (%s)\033[0m\n",
                __func__, texture_id, model_filepath);
        goto error;
    }

    entptr = &gst->enemies[gst->num_enemies];
    entptr->type = enemy_type;

    entptr->position = initial_position; 
    entptr->hitbox_size = hitbox_size;
    entptr->hitbox_position = hitbox_position;
    entptr->health = max_health;
    entptr->max_health = max_health;

    entptr->knockback_velocity = (Vector3){0};
    entptr->hit_direction = (Vector3){0};
    entptr->rotation_from_hit = (Vector3){0};
    entptr->stun_timer = 0.0;
    entptr->max_stun_time = 0.0;

    entptr->Q0 = QuaternionIdentity();
    entptr->Q1 = QuaternionIdentity();
    entptr->angle_change = 0.0;
    entptr->forward_angle = 0.0;
    
    entptr->target_range = target_range;
    entptr->has_target = 0;
    entptr->body_matrix = MatrixIdentity();
    entptr->gun_index = 0;
    entptr->index = gst->num_enemies;

    entptr->weaponptr = weaponptr;
    entptr->weapon_psysptr = weapon_psysptr;

    entptr->travel = (struct enemy_travel_t) {
        .start = (Vector3){0},
        .dest  = (Vector3){0},
        .travelled = 0.0,

        // Next enemy update will set start and dest values
        // If travelling is enabled.
        .dest_reached = 1
    };


    entptr->model = LoadModel(model_filepath);
    entptr->model.transform 
        = MatrixTranslate(initial_position.x, initial_position.y, initial_position.z);

    entptr->model.materials[0] = LoadMaterialDefault();
    entptr->model.materials[0].shader = gst->shaders[DEFAULT_SHADER];
    entptr->model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = gst->textures[texture_id];

    /*
    entptr->model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = gst->textures[texture_id];
    entptr->model.materials[0].shader = gst->shaders[DEFAULT_SHADER];

    */

    gst->num_enemies++;

    printf("\033[32m >> enemy created model filepath: '%s'\033[0m\n",
            model_filepath);

    entptr->firerate_timer = 0.0;
    entptr->firerate = firerate;

    switch(entptr->type)
    {
        case ENEMY_TYPE_LVL0:
            //entptr->weapon = &gst->enemy_weapons[ENTWEAPON_LVL0];
            enemy_lvl0_created(gst, entptr);
            break;

        // ...
    }

error:
    return entptr;
}

// just unloads the model and sets health to 0
void delete_enemy(struct enemy_t* ent) {
    UnloadModel(ent->model);
    ent->health = 0;
    ent->weapon_psysptr = NULL;
    ent->weaponptr = NULL;
}


void update_enemy(struct state_t* gst, struct enemy_t* ent) {
    switch(ent->type)
    {
        case ENEMY_TYPE_LVL0:
            enemy_lvl0_update(gst, ent);
            break;


        // ...
    }


}

void render_enemy(struct state_t* gst, struct enemy_t* ent) {
    switch(ent->type)
    {
        case ENEMY_TYPE_LVL0:
            enemy_lvl0_render(gst, ent);
            break;


        // ...
    }


}


    
void enemy_hit(struct state_t* gst, struct enemy_t* ent, struct weapon_t* weapon, 
        Vector3 hit_position, Vector3 hit_direction) {

    int was_critical_hit = 0;
    ent->health -= get_weapon_damage(weapon, &was_critical_hit);

    if(was_critical_hit) {
        state_add_crithit_marker(gst, hit_position);
    }

    if(ent->health <= 0.001) {
        ent->health = 0.0;
        enemy_death(gst, ent);
        return;
    }

    switch(ent->type)
    {
        case ENEMY_TYPE_LVL0:
            enemy_lvl0_hit(gst, ent, hit_position, hit_direction);
            break;


        // ...
    }

}

void enemy_death(struct state_t* gst, struct enemy_t* ent) {

    switch(ent->type)
    {
        case ENEMY_TYPE_LVL0:
            enemy_lvl0_death(gst, ent);
            break;


        // ...
    }

   
}


BoundingBox get_enemy_boundingbox(struct enemy_t* ent) {
    return (BoundingBox) {
        (Vector3) {
            // Minimum box corner
            (ent->position.x + ent->hitbox_position.x) - ent->hitbox_size.x/2,
            (ent->position.y + ent->hitbox_position.y) - ent->hitbox_size.y/2,
            (ent->position.z + ent->hitbox_position.z) - ent->hitbox_size.z/2
        },
        (Vector3) {
            // Maximum box corner
            (ent->position.x + ent->hitbox_position.x) + ent->hitbox_size.x/2,
            (ent->position.y + ent->hitbox_position.y) + ent->hitbox_size.y/2,
            (ent->position.z + ent->hitbox_position.z) + ent->hitbox_size.z/2
        }
    };
}

