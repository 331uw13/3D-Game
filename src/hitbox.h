#ifndef HITBOX_H
#define HITBOX_H

#include <raylib.h>
#include <stddef.h>

struct hitbox_t {
    Vector3 size;
    Vector3 offset;
    float damage_mult;
};


// Returns pointer to the hitbox that was collided with 'boundingbox'
// or NULL if no collision.
int check_collision_hitboxes(BoundingBox* boundingbox, struct hitbox_t* hitboxes, size_t num_hitboxes);




#endif
