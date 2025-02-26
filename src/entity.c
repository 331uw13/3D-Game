#include <stdio.h>

#include "state.h"
#include "entity.h"
#include "util.h"

#include "enemies/enemy_lvl0.h"



// Returns pointer into gst->entities array where new entity was created if successful.
struct entity_t* create_entity(
        struct state_t* gst,
        int entity_type,
        int entity_travel_enabled,
        const char* model_filepath,
        int texture_id,
        int max_health,
        Vector3 initial_position,
        Vector3 hitbox_size,
        float target_range,
        float firerate
){
    struct entity_t* entptr = NULL;

    if((gst->num_entities+1) >= MAX_ENTITIES) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Max entities reached.\033[0m\n",
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

    entptr = &gst->entities[gst->num_entities];
    entptr->type = entity_type;

    entptr->position = initial_position;
    entptr->hitbox_size = hitbox_size;

    entptr->health = max_health;
    entptr->max_health = max_health;

    entptr->knockback_velocity = (Vector3){0};
    entptr->hit_direction = (Vector3){0};
    entptr->rotation_from_hit = (Vector3){0};
    entptr->was_hit = 0;

    entptr->Q0 = QuaternionIdentity();
    entptr->Q1 = QuaternionIdentity();
    entptr->angle_change = 0.0;
    entptr->forward_angle = 0.0;
    
    entptr->target_range = target_range;
    entptr->has_target = 0;
    entptr->body_matrix = MatrixIdentity();
    entptr->gun_index = 0;
        
    entptr->travel = (struct entity_travel_t) {
        .start = (Vector3){0},
        .dest  = (Vector3){0},
        .travelled = 0.0,

        // Next entity update will set start and dest values
        // If travelling is enabled.
        .dest_reached = 1,
        .enabled = entity_travel_enabled
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

    gst->num_entities++;

    printf("\033[32m >> Entity created model filepath: '%s'\033[0m\n",
            model_filepath);

    entptr->firerate_timer = 0.0;
    entptr->firerate = firerate;

    switch(entptr->type)
    {
        case ENT_TYPE_LVL0:
            entptr->weapon = &gst->entity_weapons[ENTWEAPON_LVL0];
            enemy_lvl0_created(gst, entptr);
            break;

        // ...
    }

error:
    return entptr;
}

// just unloads the model and sets health to 0
void delete_entity(struct entity_t* ent) {
    UnloadModel(ent->model);
    ent->health = 0;
    ent->weapon = NULL;
}


void update_entity(struct state_t* gst, struct entity_t* ent) {
    switch(ent->type)
    {
        case ENT_TYPE_LVL0:
            enemy_lvl0_update(gst, ent);
            break;


        // ...
    }


}

void render_entity(struct state_t* gst, struct entity_t* ent) {
    switch(ent->type)
    {
        case ENT_TYPE_LVL0:
            enemy_lvl0_render(gst, ent);
            break;


        // ...
    }


}

void entity_hit(struct state_t* gst, struct entity_t* ent) {
}




