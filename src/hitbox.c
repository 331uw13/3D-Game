#include "hitbox.h"

/*


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

   */
int check_collision_hitboxes(BoundingBox* boundingbox, struct hitbox_t* hitboxes, size_t num_hitboxes) {
    int result = 0;
    if(!hitboxes || !boundingbox) {
        goto error;
    }




error:
    return result;
}



