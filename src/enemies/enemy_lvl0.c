
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


    RayCollision ray;
    Matrix legs_rotation = get_rotation_to_surface(&gst->terrain, ent->position.x, ent->position.z, &ray);    
    Matrix translation = MatrixTranslate(ent->position.x, ray.point.y, ent->position.z);




    ent->matrix[ENEMY_LVL0_LEG_MI] = MatrixMultiply(legs_rotation, translation);
    ent->matrix[ENEMY_LVL0_JOINT_MI] = ent->matrix[ENEMY_LVL0_LEG_MI];
    ent->matrix[ENEMY_LVL0_BODY_MI] = translation;
    
    ent->position.y = ray.point.y;


    if(ent->state != ENT_STATE_CHANGING_ANGLE) {
        set_body_transform(ent, ray.normal);
    }
    
    int infov = player_in_enemy_fov(gst, ent, &ent->matrix[ENEMY_LVL0_BODY_MI]);
    // ^- Remember to set the body transformation matrix first
   

    int has_target_now = enemy_can_see_player(gst, ent);




    if((ent->mood == ENT_HOSTILE) && infov && (has_target_now && !ent->has_target)) {
        printf("Enemy(%li) -> Target Found!\n", ent->index);
        ent->state = ENT_STATE_CHANGING_ANGLE;
    
        // Get (current) quaternion.
        ent->Q_prev = QuaternionFromMatrix(ent->matrix[ENEMY_LVL0_BODY_MI]);

        // Get the (target) quaternion.
        ent->rotation = get_rotation_yz(ent->position, gst->player.position);
        set_body_transform(ent, ray.normal);
        ent->Q_target = QuaternionFromMatrix(ent->matrix[ENEMY_LVL0_BODY_MI]);

        ent->angle_change = 0.0;
        ent->has_target = 1;
    }
    else
    if(!has_target_now && ent->has_target) {
        printf("Enemy(%li) -> Target Lost.\n", ent->index);
        ent->state = ENT_STATE_SEARCHING_TARGET;
        
        ent->Q_rnd_target = QuaternionFromMatrix(ent->matrix[ENEMY_LVL0_BODY_MI]);

        ent->rnd_angle_change = 0.0;
        ent->has_target = 0;
    }
    /*

 199 // Draw a circle in 3D world space
 200 void DrawCircle3D(Vector3 center, float radius, Vector3 rotationAxis, float rotationAngle, Color color)
 201 {
 202     rlPushMatrix();
 203         rlTranslatef(center.x, center.y, center.z);
 204         rlRotatef(rotationAngle, rotationAxis.x, rotationAxis.y, rotationAxis.z);
 205
 206         rlBegin(RL_LINES);
 207             for (int i = 0; i < 360; i += 10)
 208             {
 209                 rlColor4ub(color.r, color.g, color.b, color.a);
 210
 211                 rlVertex3f(sinf(DEG2RAD*i)*radius, cosf(DEG2RAD*i)*radius, 0.0f);
 212                 rlVertex3f(sinf(DEG2RAD*(i + 10))*radius, cosf(DEG2RAD*(i + 10))*radius, 0.0f);
 213             }
 214         rlEnd();
 215     rlPopMatrix();
 216 }
    
    // Vector3 Vector3RotateByAxisAngle(Vector3 v, Vector3 axis, float angle)
    

     */
    switch(ent->state) {
        case ENT_STATE_HAS_TARGET:
            {
                ent->rotation = get_rotation_yz(ent->position, gst->player.position);


                if(ent->firerate_timer >= ent->firerate) {

                    Vector3 prj_pos = (Vector3) {
                        ent->position.x + cos(-ent->rotation.y + M_PI/2) * 10.0,
                        ent->position.y+10.0,
                        ent->position.z + sin(-ent->rotation.y + M_PI/2) * 10.0
                    };

                    Vector3 prj_dir = Vector3Normalize(Vector3Subtract(gst->player.position, prj_pos));
                

                    add_projectile(gst, 
                            &gst->psystems[ENEMY_LVL0_WEAPON_PSYS],
                            &gst->enemy_weapons[ENEMY_LVL0_WEAPON], 
                            prj_pos, prj_dir, 0);

                    ent->firerate_timer = 0.0;
                }
            }
            break;

        case ENT_STATE_CHANGING_ANGLE:
            {
                const float duration = 0.2;
                Quaternion Q = QuaternionSlerp(ent->Q_prev, ent->Q_target, ent->angle_change / duration);

                ent->angle_change += gst->dt;

                if(ent->angle_change >= duration) {
                    ent->state = ENT_STATE_HAS_TARGET;
                }

                ent->matrix[ENEMY_LVL0_BODY_MI]
                    = MatrixTranslate(ray.normal.x*8.0, 10.0, ray.normal.z*8.0);
                
                ent->matrix[ENEMY_LVL0_BODY_MI] 
                    = MatrixMultiply(QuaternionToMatrix(Q), ent->matrix[ENEMY_LVL0_BODY_MI]);
            }
            break;

        case ENT_STATE_SEARCHING_TARGET:
            {
                if(ent->rnd_angle_change <= 0.00) {
                    // Decide random point to rotate towards.

                    ent->Q_rnd_prev = ent->Q_rnd_target;


                    ent->rotation = (Vector3){ 
                        0, // Ignore X (roll)
                        RSEEDRANDOMF(-M_PI, M_PI),
                        RSEEDRANDOMF(-0.3, 0.2)
                    };

                    set_body_transform(ent, ray.normal);
                    ent->Q_rnd_target = QuaternionFromMatrix(ent->matrix[ENEMY_LVL0_BODY_MI]);
                }

                const float duration = 2.0;
                Quaternion Q 
                    = QuaternionSlerp(ent->Q_rnd_prev, ent->Q_rnd_target, ent->rnd_angle_change / duration);

                ent->rnd_angle_change += gst->dt;
                if(ent->rnd_angle_change >= duration) {
                    ent->rnd_angle_change = 0.0;
                }
                
                ent->matrix[ENEMY_LVL0_BODY_MI]
                    = MatrixTranslate(ray.normal.x*8.0, 10.0, ray.normal.z*8.0);
                
                ent->matrix[ENEMY_LVL0_BODY_MI] 
                    = MatrixMultiply(QuaternionToMatrix(Q), ent->matrix[ENEMY_LVL0_BODY_MI]);
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

    // Update the hitbox offsets, they may have changed with terrain surface normal.

    RayCollision ray = raycast_terrain(&gst->terrain, ent->position.x, ent->position.z);

    for(size_t i = 0; i < ent->num_hitboxes; i++) {
        ent->hitboxes[i].offset.x += ray.normal.x*8;
        ent->hitboxes[i].offset.z += ray.normal.z*8;
    }

}

