#ifndef GAME_HITBOX_H
#define GAME_HITBOX_H

#include <raylib.h>


// Hitbox tag
#define HITBOX_LEGS 0
#define HITBOX_BODY 1
#define HITBOX_HEAD 2



// TODO: Add transformation?


struct hitbox_t {
    Vector3 size;
    Vector3 offset;
    float   damage_mult;
    int     hits;
    int     tag;
};

struct enemy_t;


// Returns pointer to the hitbox that was collided with 'boundingbox'
// or NULL if no collision.
struct hitbox_t* check_collision_hitboxes(const BoundingBox* boundingbox, struct enemy_t* ent);


// More reliable way to check for collisions for fast moving objects. 
// A to B
// Mainly used for projectiles.
int raycast_hitbox(
        Vector3 hitbox_owner_pos,
        struct hitbox_t* hitbox,
        Vector3 pointA,
        Vector3 pointB,
        float point_radius);






#endif
