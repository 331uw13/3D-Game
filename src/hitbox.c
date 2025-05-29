


#include "hitbox.h"
#include "state/state.h"






struct hitbox_t* check_collision_hitboxes(const BoundingBox* boundingbox, struct enemy_t* ent) {
    struct hitbox_t* result = NULL;

    for(size_t i = 0; i < ent->num_hitboxes; i++) {
        BoundingBox hitbox = (BoundingBox) {
            (Vector3) {
                // Minimum box corner
                (ent->position.x + ent->hitboxes[i].offset.x) - ent->hitboxes[i].size.x/2,
                (ent->position.y + ent->hitboxes[i].offset.y) - ent->hitboxes[i].size.y/2,
                (ent->position.z + ent->hitboxes[i].offset.z) - ent->hitboxes[i].size.z/2,
            },
            (Vector3) {
                // Maximum box corner
                (ent->position.x + ent->hitboxes[i].offset.x) + ent->hitboxes[i].size.x/2,
                (ent->position.y + ent->hitboxes[i].offset.y) + ent->hitboxes[i].size.y/2,
                (ent->position.z + ent->hitboxes[i].offset.z) + ent->hitboxes[i].size.z/2,
            }
        };

        if(CheckCollisionBoxes(*boundingbox, hitbox)) {
            result = &ent->hitboxes[i];
            break;
        }
    }

    return result;
}


int raycast_hitbox(Vector3 hitbox_owner_pos, struct hitbox_t* hitbox, Vector3 pointA, Vector3 pointB, float point_radius) {
    int result = 0;

    const float max_dist = Vector3Distance(pointA, pointB);
    
    
    Ray ray = (Ray) {
        .position = pointA,
        .direction = Vector3Normalize(Vector3Subtract(pointA, pointB))
    };


    BoundingBox boundingbox = (BoundingBox) {
        (Vector3) {
            // Minimum box corner
            (hitbox_owner_pos.x + hitbox->offset.x) - hitbox->size.x/2,
            (hitbox_owner_pos.y + hitbox->offset.y) - hitbox->size.y/2,
            (hitbox_owner_pos.z + hitbox->offset.z) - hitbox->size.z/2
        },
        (Vector3) {
            // Maximum box corner
            (hitbox_owner_pos.x + hitbox->offset.x) + hitbox->size.x/2,
            (hitbox_owner_pos.y + hitbox->offset.y) + hitbox->size.y/2,
            (hitbox_owner_pos.z + hitbox->offset.z) + hitbox->size.z/2
        }
    };

    RayCollision rc = GetRayCollisionBox(ray, boundingbox);
    
    result = rc.hit && (max_dist < rc.distance);

    return result;
}


