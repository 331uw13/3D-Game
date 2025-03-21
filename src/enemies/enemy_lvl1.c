
#include "enemy_lvl1.h"
#include "../state.h"
#include "../util.h"

#include <raymath.h>
#include <stdio.h>

#include <stdlib.h>


#define HOVER_YLEVEL 100.0
#define NEW_RND_DEST_RADIUS 150


static void pick_new_random_travel_dest(struct state_t* gst, struct enemy_t* ent) {


    ent->travel.start = ent->position;
    ent->travel.dest = (Vector3){
        ent->position.x + RSEEDRANDOMF(-NEW_RND_DEST_RADIUS, NEW_RND_DEST_RADIUS),
            0, // Y is ignored.
        ent->position.z + RSEEDRANDOMF(-NEW_RND_DEST_RADIUS, NEW_RND_DEST_RADIUS)
    };

    ent->travel.dest_reached = 0;
    ent->travel.travelled = 0;

}


void enemy_lvl1_update(struct state_t* gst, struct enemy_t* ent) {
    if(!ent->alive) {
        return;
    }


    RayCollision ray = raycast_terrain(&gst->terrain, ent->position.x, ent->position.z);
    float ypoint = CLAMP(ray.point.y, gst->terrain.water_ylevel, 1000.0);
    ent->position.y = ypoint + HOVER_YLEVEL;
    
    Matrix translation = MatrixTranslate(ent->position.x, ent->position.y, ent->position.z);


    ent->matrix[ENEMY_LVL1_BODY_MI] = QuaternionToMatrix(ent->Q_now);


    switch(ent->state) {
        case ENT_STATE_HAS_TARGET:
            {

            }
            break;

        case ENT_STATE_CHANGING_ANGLE:
            {
            }
            break;

        case ENT_STATE_SEARCHING_TARGET:
            {

                // Smooth start to dest.
                float t = 0.5+0.5*sin((ent->travel.travelled) - (M_PI/2.0));
                Vector3 tpos = Vector3Lerp(ent->travel.start, ent->travel.dest, t);

                ent->travel.travelled += gst->dt;

                if(Vector3Distance(
                            (Vector3){ ent->position.x, 0, ent->position.z },
                            ent->travel.dest) <= 3.0) {
                    printf("dest_reached\n");
                    ent->travel.dest_reached = 1;
                }

                if(ent->travel.dest_reached) {
                    pick_new_random_travel_dest(gst, ent);

                    ent->Q_prev = ent->Q_now;
                    ent->Q_target = QuaternionFromMatrix(
                                        MatrixRotateY(angle_xz(ent->travel.start, ent->travel.dest)));
                    ent->angle_change = 0.0;
                }

                ent->position.x = tpos.x;
                ent->position.z = tpos.z;

                const float d = 0.5;

                ent->Q_now = QuaternionSlerp(ent->Q_prev, ent->Q_target, ent->angle_change/d);
                if(ent->angle_change < d) {
                    ent->angle_change += gst->dt;
                }

            }
            break;

    }

    ent->matrix[ENEMY_LVL1_BODY_MI]
        = MatrixMultiply(ent->matrix[ENEMY_LVL1_BODY_MI], translation);

}

void enemy_lvl1_render(struct state_t* gst, struct enemy_t* ent) {
    if(!ent->alive) {
        return;
    }

    DrawMesh(ent->modelptr->meshes[0],
             ent->modelptr->materials[0],
             ent->matrix[ENEMY_LVL1_BODY_MI]);

}


void enemy_lvl1_hit(struct state_t* gst, struct enemy_t* ent,
        Vector3 hit_position, Vector3 hit_direction) {
   
    ent->knockback_velocity = Vector3Scale(hit_direction, 5.0);


    ent->max_stun_time = 0.5;
    ent->stun_timer = 0.0;
    
    ent->previous_state = ent->state;
    ent->state = ENT_STATE_WASHIT;
}

void enemy_lvl1_death(struct state_t* gst, struct enemy_t* ent) {
    printf("(INFO) '%s': Enemy %li Died\n", __func__, ent->index);
    
}

void enemy_lvl1_created(struct state_t* gst, struct enemy_t* ent) {
    ent->state = ENT_STATE_SEARCHING_TARGET;
    ent->Q_now = QuaternionFromMatrix(MatrixRotateY(0));
}

