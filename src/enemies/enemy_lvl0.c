
#include "enemy_lvl0.h"
#include "../state.h"
#include "../util.h"

#include <raymath.h>
#include <stdio.h>

#include <stdlib.h>


static void set_body_transform(struct enemy_t* ent, Vector3 tnormal) {
    
    const float norm_off = 8.0; // How much the surface normal offsets the body position?
    const float body_yoff = 10.0; // How much to offset the body Y position?
   
    // Translate to wanted position.
    ent->matrix[ENEMY_LVL0_BODY_MI]
            = MatrixTranslate(
                    tnormal.x * norm_off,
                    body_yoff,
                    tnormal.z * norm_off
                    );

    // Rotate angles.

    ent->matrix[ENEMY_LVL0_BODY_MI] 
        = MatrixMultiply(MatrixRotateY(ent->rotation.y), ent->matrix[ENEMY_LVL0_BODY_MI]);

    ent->matrix[ENEMY_LVL0_BODY_MI]
        = MatrixMultiply(MatrixRotateZ(ent->rotation.z), ent->matrix[ENEMY_LVL0_BODY_MI]);
}

void enemy_lvl0_update(struct state_t* gst, struct enemy_t* ent) {
    if(!ent->alive) {
        return;
    }




    RayCollision tray;
    Matrix legs_rotation = get_rotation_to_surface(&gst->terrain, ent->position.x, ent->position.z, &tray);    
    Matrix translation = MatrixTranslate(ent->position.x, tray.point.y, ent->position.z);


    ent->matrix[ENEMY_LVL0_LEG_MI] = MatrixMultiply(legs_rotation, translation);
    ent->matrix[ENEMY_LVL0_JOINT_MI] = ent->matrix[ENEMY_LVL0_LEG_MI];
    ent->matrix[ENEMY_LVL0_BODY_MI] = translation;
    
    ent->position.y = tray.point.y;


    // TODO: get distance to player for enemy struct.

    //float dst2player = Vector3Distance(ent->position, gst->player.position);
    int has_target_now = enemy_can_see_player(gst, ent);



    if(ent->state != ENT_STATE_CHANGING_ANGLE) {
        set_body_transform(ent, tray.normal);
    }


    if(has_target_now && !ent->has_target) {
        printf(" Enemy(%li) -> Target Found!\n", ent->index);
        ent->state = ENT_STATE_CHANGING_ANGLE;
    
        // Get (current) quaternion.
        ent->Q_prev = QuaternionFromMatrix(ent->matrix[ENEMY_LVL0_BODY_MI]);

        // Get the (target) quaternion.
        ent->rotation = get_rotation_yz(ent->position, gst->player.position);
        set_body_transform(ent, tray.normal);
        ent->Q_target = QuaternionFromMatrix(ent->matrix[ENEMY_LVL0_BODY_MI]);

        ent->angle_change = 0.0;
    }
    else
    if(!has_target_now && ent->has_target) {
        printf(" Enemy(%li) -> Target Lost.\n", ent->index);
        ent->state = ENT_STATE_SEARCHING_TARGET;
    
    }
    
    ent->has_target = has_target_now;



    {
        Vector3 forward = (Vector3){ 0.0, 0.0, 0.0 };
        forward.x = ent->matrix[ENEMY_LVL0_BODY_MI].m8;
        forward.y = ent->matrix[ENEMY_LVL0_BODY_MI].m9;
        forward.z = ent->matrix[ENEMY_LVL0_BODY_MI].m10;

        forward = Vector3CrossProduct(forward, (Vector3){ 0.0, 1.0, 0.0 });

        Vector3 P1 = (Vector3) {
            ent->position.x,
                0,
            ent->position.z
        };

        Vector3 P2 = (Vector3) {
            gst->player.position.x,
                0,
            gst->player.position.z
        };
        Vector3 D = Vector3Normalize(Vector3Subtract(P1, P2));

        float dot = Vector3DotProduct(D, forward);

        printf("dotprod:%f\n", dot);

    }
    switch(ent->state) {
        case ENT_STATE_HAS_TARGET:
            {
                ent->rotation = get_rotation_yz(ent->position, gst->player.position);

                

            }
            break;

        case ENT_STATE_CHANGING_ANGLE:
            {
                Quaternion Q = QuaternionSlerp(ent->Q_prev, ent->Q_target, ent->angle_change);

                ent->angle_change += gst->dt * 5.0;

                if(ent->angle_change >= 1.0) {
                    ent->state = ENT_STATE_HAS_TARGET;
                }

                ent->matrix[ENEMY_LVL0_BODY_MI]
                    = MatrixTranslate(tray.normal.x*8.0, 10.0, tray.normal.z*8.0);
                
                ent->matrix[ENEMY_LVL0_BODY_MI] 
                    = MatrixMultiply(QuaternionToMatrix(Q), ent->matrix[ENEMY_LVL0_BODY_MI]);

            }
            break;

        case ENT_STATE_SEARCHING_TARGET:
            {
                
            }
            break;

    }

  
    ent->matrix[ENEMY_LVL0_BODY_MI]
        = MatrixMultiply(ent->matrix[ENEMY_LVL0_BODY_MI], translation);

}

void enemy_lvl0_render(struct state_t* gst, struct enemy_t* ent) {
    if(!ent->alive) {
        return;
    }

    // Turret body
    DrawMesh(ent->modelptr->meshes[0],
             ent->modelptr->materials[0],
             ent->matrix[ENEMY_LVL0_BODY_MI]);

    // Turret body center joint
    DrawMesh(ent->modelptr->meshes[2],
             ent->modelptr->materials[0],
             ent->matrix[ENEMY_LVL0_JOINT_MI]);

    // Turret legs
    DrawMesh(ent->modelptr->meshes[1],
             ent->modelptr->materials[0],
             ent->matrix[ENEMY_LVL0_LEG_MI]);
    
}


void enemy_lvl0_hit(struct state_t* gst, struct enemy_t* ent,
        Vector3 hit_position, Vector3 hit_direction) {
   
    ent->knockback_velocity = Vector3Scale(hit_direction, 5.0);


    ent->max_stun_time = 0.5;
    ent->stun_timer = 0.0;
    
    ent->previous_state = ent->state;
    ent->state = ENT_STATE_WASHIT;
}

void enemy_lvl0_death(struct state_t* gst, struct enemy_t* ent) {
    printf("(INFO) '%s': Enemy %li Died\n", __func__, ent->index);
    
}

void enemy_lvl0_created(struct state_t* gst, struct enemy_t* ent) {
    ent->state = ENT_STATE_SEARCHING_TARGET;

}

