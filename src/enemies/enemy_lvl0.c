
#include "enemy_lvl0.h"
#include "../state.h"
#include "../util.h"

#include <raymath.h>
#include <stdio.h>

#include <stdlib.h>


void enemy_lvl0_update(struct state_t* gst, struct enemy_t* ent) {
    if(!ent->alive) {
        return;
    }


    float terrain_y = 0.0;
    Matrix legs_rotation = get_rotation_to_surface(&gst->terrain, ent->position.x, ent->position.z, &terrain_y);    
    Matrix translation = MatrixTranslate(ent->position.x, terrain_y, ent->position.z);



    ent->matrix[ENEMY_LVL0_LEG_MI] = MatrixMultiply(legs_rotation, translation);
    ent->matrix[ENEMY_LVL0_BODY_MI] = translation;
    ent->matrix[ENEMY_LVL0_JOINT_MI] = ent->matrix[ENEMY_LVL0_LEG_MI];
    ent->position.y = terrain_y;


    //float dst2player = Vector3Distance(ent->position, gst->player.position);
    int has_target_now = enemy_can_see_player(gst, ent);



    if(has_target_now && !ent->has_target) {
        printf(" Enemy(%li) -> Target Found!\n", ent->index);
        ent->state = ENT_STATE_HAS_TARGET;
        /*
        ent->angle_change = 0.0;
        Matrix prev_angle_m = MatrixRotateY(ent->forward_angle);

        ent->Q1 = QuaternionFromMatrix(prev_angle_m);
        ent->forward_angle = angle_xz(gst->player.position, ent->position);

        Matrix rm = MatrixRotateY(ent->forward_angle);
        ent->Q0 = QuaternionFromMatrix(rm);
        */
    }
    else
    if(!has_target_now && ent->has_target) {
        printf(" Enemy(%li) -> Target Lost.\n", ent->index);
        ent->state = ENT_STATE_SEARCHING_TARGET;
    
    }
    
    ent->has_target = has_target_now;



    switch(ent->state) {
        case ENT_STATE_HAS_TARGET:
            {
                ent->forward_angle = angle_xz(gst->player.position, ent->position);
                ent->matrix[ENEMY_LVL0_BODY_MI] = MatrixRotateY(ent->forward_angle);

                Vector3 P1 = gst->player.position;
                Vector3 P2 = ent->position;

                Vector3 d = Vector3Subtract(P1, P2);
                float angle = acos(d.y / Vector3Length(d));


                // TODO: get the normal at top.

                RayCollision ray = raycast_terrain(&gst->terrain, ent->position.x, ent->position.z);
                ray.point = Vector3Add(ray.point, ray.normal);


                // Translate to origin.
                ent->matrix[ENEMY_LVL0_BODY_MI] = MatrixTranslate(
                        ray.normal.x*10,
                        10.0,
                        ray.normal.z*10
                        );
                
                // Rotate 'X,Z'
                ent->matrix[ENEMY_LVL0_BODY_MI]
                    = MatrixMultiply(MatrixRotateY(ent->forward_angle), ent->matrix[ENEMY_LVL0_BODY_MI]);

                // Rotate 'Y'
                ent->matrix[ENEMY_LVL0_BODY_MI]
                    = MatrixMultiply(MatrixRotateZ(angle - 90.0*DEG2RAD), ent->matrix[ENEMY_LVL0_BODY_MI]);
              

                // Translate back

            }
            break;

        case ENT_STATE_SEARCHING_TARGET:
            {
                ent->forward_angle += gst->dt;
                ent->matrix[ENEMY_LVL0_BODY_MI] = MatrixRotateY(ent->forward_angle);
            }
            break;
    }

                ent->matrix[ENEMY_LVL0_BODY_MI]
                    = MatrixMultiply(ent->matrix[ENEMY_LVL0_BODY_MI], translation);



  
    /*
    ent->matrix[ENEMY_LVL0_BODY_MI] 
        = MatrixMultiply(ent->matrix[ENEMY_LVL0_BODY_MI], translation);

        */
    
    /*
    if(ent->firerate_timer < ent->firerate) {
        ent->firerate_timer += gst->dt;
    }

    switch(ent->state) {

        case ENT_STATE_WASHIT:
            {
                ent->stun_timer += gst->dt;

                
                ent->position.x += ent->knockback_velocity.x * gst->dt;
                ent->position.z += ent->knockback_velocity.z * gst->dt;

                ent->knockback_velocity.x *= 0.99;
                ent->knockback_velocity.z *= 0.99;


                if(ent->stun_timer >= ent->max_stun_time) {
                    ent->state = ENT_STATE_SEARCHING_TARGET;
                    ent->has_target = 0;
                }
                
                Matrix rm = MatrixRotateY(ent->forward_angle);
                rm = MatrixMultiply(rm, ent->matrix[ENEMY_LVL0_BODY_MI]);
                ent->matrix[ENEMY_LVL0_BODY_MI] = rm;
            }
            break;
        case ENT_STATE_SEARCHING_TARGET:
            {
                ent->forward_angle += gst->dt;
                Matrix rm = MatrixRotateY(ent->forward_angle);
                rm = MatrixMultiply(rm, ent->matrix[ENEMY_LVL0_BODY_MI]);
                ent->matrix[ENEMY_LVL0_BODY_MI] = rm;
            }
            break;

        case ENT_STATE_CHANGING_ANGLE:
            {
                Quaternion Q = QuaternionSlerp(ent->Q1, ent->Q0, ent->angle_change);
                ent->angle_change += gst->dt * 5.0;

                if(ent->angle_change > 1.0) {
                    ent->state = ENT_STATE_HAS_TARGET;
                }

                Matrix qm = QuaternionToMatrix(Q);
                ent->matrix[ENEMY_LVL0_BODY_MI] = MatrixMultiply(qm, ent->matrix[ENEMY_LVL0_BODY_MI]);
            }
            break;

        case ENT_STATE_HAS_TARGET:
            {
                ent->forward_angle = angle_xz(gst->player.position, ent->position);
                Matrix rm = MatrixRotateY(ent->forward_angle);
                rm = MatrixMultiply(rm, ent->matrix[ENEMY_LVL0_BODY_MI]);
                ent->matrix[ENEMY_LVL0_BODY_MI] = rm;
    
                Vector3 prj_position = ent->position;


                float angle = ent->forward_angle;
                const float ang_rad = 1.5;


                if(!gst->player.noclip && gst->player.alive) {
                    prj_position.x += 0.2;
                    prj_position.z -= 0.4;

                    if(ent->gun_index) {
                        prj_position.x += ang_rad*sin(angle)*2;
                        prj_position.z += ang_rad*cos(angle)*2;
                    }
                    else {
                        prj_position.x -= ang_rad*sin(angle)*2;
                        prj_position.z -= ang_rad*cos(angle)*2;
                    }
                   
                    prj_position.y += 3.5;
                    Vector3 prj_direction = Vector3Normalize(Vector3Subtract(gst->player.position, prj_position));


                    if(ent->firerate_timer >= ent->firerate) {
                        
                        add_projectile(gst, ent->weapon_psysptr, ent->weaponptr,
                                prj_position, prj_direction, NO_ACCURACY_MOD);
                        
                        ent->firerate_timer = 0.0;
                        ent->gun_index = !ent->gun_index;

                        SetSoundVolume(gst->sounds[ENEMY_GUN_SOUND],
                                get_volume_dist(gst->player.position, ent->position));
                        
                        PlaySound(gst->sounds[ENEMY_GUN_SOUND]);
                    }
                }
            }
            break;
    }
*/
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

