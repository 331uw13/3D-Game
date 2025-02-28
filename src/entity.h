#ifndef ENTITY_H
#define ENTITY_H

#include <raylib.h>
#include <stddef.h>

#define ENT_STATE_IDLE 0
#define ENT_STATE_SEARCHING_TARGET 1
#define ENT_STATE_HAS_TARGET 2
#define ENT_STATE_CHANGING_ANGLE 3

#define ENT_TRAVELING_DISABLED 0
#define ENT_TRAVELING_ENABLED 1

#define ENT_TYPE_LVL0 0
#define ENT_TYPE_LVL1 1


// This handles all basic behaviour for enemies.
// Then calls 'enemies/enemy_lvl*.c'(depending on "entity type") 
// to handle the rest if needed

#include "weapon.h"

struct state_t;


struct entity_travel_t {
    Vector3 start; // Start position
    Vector3 dest;  // Destination position.
    float travelled; // Used for linear interpolation.

    int dest_reached;
    int enabled; // Some entities may not move.
};

struct entity_t {

    Model model;
    
    int type;

    Vector3 position; // <- NOTE: "read only". modify the model's transform instead.
    Vector3 hitbox_size; // TODO: multiple hitboxes.
    Vector3 hitbox_position; // hitbox position from 'entity position'.
    Matrix body_matrix; // Some entities have rotating body and 'model.transform' is used for legs etc.


    float target_range; // how far can the entity "see" the player
    int   has_target;

    float health;
    float max_health;

    // When entity gets hit. velocity is applied.
    Vector3 knockback_velocity;
    Vector3 hit_direction;
    
    // Rotation is applied when hit.
    Vector3 rotation_from_hit;

    int was_hit;
    
    // Used for rotating angles.
    Quaternion Q0;
    Quaternion Q1;
    float      angle_change; // how much angle is changed to another. 0.0 to 1.0

    float forward_angle;

    // For any kind of movement entity has.
    struct entity_travel_t travel;

    // Used for selecting new point where to travel
    // when state is 'SEARCHING_TARGET'
    float rnd_search_radius;
    float rnd_search_min_radius;

    int state;
    size_t index; // index in gst->entities array.

    struct weapon_t* weapon;
    float firerate;
    float firerate_timer;
    int gun_index; // switch between model's guns.
};

// Probably not going to have ALOT of enemies at once.
// So just leaving the element not used at its index
// And when updating all entities, looping through the whole array
// And updating only alive ones will be faster than constanty shifting the array back and forth.


// TODO: do this same way than in weapon.c
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
        Vector3 hitbox_position, // (from origin)
        float target_range,
        float firerate
        );

// just unloads the model and sets health to 0
void delete_entity(struct entity_t* ent);

// "Render settings"
#define ENT_UPDATE_ONLY 0
#define ENT_RENDER_ON_UPDATE 1

// These functions "redirects" the call based on entity type
void update_entity(struct state_t* gst, struct entity_t* ent);
void render_entity(struct state_t* gst, struct entity_t* ent);
void entity_hit(struct state_t* gst, struct entity_t* ent);
void entity_death(struct state_t* gst, struct entity_t* ent);

BoundingBox get_entity_boundingbox(struct entity_t* ent);




#endif
