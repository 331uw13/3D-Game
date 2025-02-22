#include <stdio.h>

#include "state.h"
#include "entity.h"
#include "util.h"




// Returns pointer into gst->entities array where new entity was created if successful.
struct entity_t* setup_entity(
        struct state_t* gst,
        int entity_type,
        int entity_travel_enabled,
        const char* model_filepath,
        size_t texture_id,
        int max_health,
        Vector3 initial_position,
        Vector3 hitbox_size
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
        fprintf(stderr, "\033[31m(ERROR) '%s': Invalid texture id: %li for (%s)\033[0m\n",
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

    entptr->forward_angle = 0.0;
    entptr->previous_angle = 0.0;
    entptr->angle_change = 0.0;

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

    entptr->model.materials[0].shader = gst->shaders[DEFAULT_SHADER];
    entptr->model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = gst->textures[texture_id];

error:
    return entptr;
}

// just unloads the model and sets health to 0
void delete_entity(struct entity_t* ent) {
    UnloadModel(ent->model);
    ent->health = 0;
}

// "Render settings"
#define ENT_UPDATE_ONLY 0
#define ENT_RENDER_ON_UPDATE 1
void update_entity(struct state_t* gst, struct entity_t* ent, int render_setting) {




}


void entity_hit(struct state_t* gst, struct entity_t* ent) {
}




