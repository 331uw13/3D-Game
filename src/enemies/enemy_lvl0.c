
#include "enemy_lvl0.h"
#include "../state.h"
#include "../util.h"

#include <raymath.h>
#include <stdio.h>




void enemy_lvl0_update(struct state_t* gst, struct enemy_t* ent) {


    if(ent->health <= 0.001) {
        return;
    }

    // Rotate to terrain surface

    float terrain_y = 0.0;
    Matrix rotation = get_rotation_to_surface(&gst->terrain, ent->position.x, ent->position.z, &terrain_y);
    Matrix translation = MatrixTranslate(ent->position.x, terrain_y, ent->position.z);

    ent->model.transform = MatrixMultiply(rotation, translation);
    ent->position.y = terrain_y;

    ent->body_matrix = ent->model.transform;


    //float dst2player = Vector3Distance(ent->position, gst->player.position);
    int has_target_now = enemy_can_see_player(gst, ent);



    if(has_target_now && !ent->has_target) {
        printf(" Enemy(%li) -> Target Found!\n", ent->index);
        ent->state = ENT_STATE_HAS_TARGET;
                
        ent->angle_change = 0.0;
        Matrix prev_angle_m = MatrixRotateY(ent->forward_angle);

        ent->Q1 = QuaternionFromMatrix(prev_angle_m);
        ent->forward_angle = angle_xz(gst->player.position, ent->position);

        Matrix rm = MatrixRotateY(ent->forward_angle);
        ent->Q0 = QuaternionFromMatrix(rm);
    }
    else
    if(!has_target_now && ent->has_target) {
        printf(" Enemy(%li) -> Target Lost.\n", ent->index);
        ent->state = ENT_STATE_SEARCHING_TARGET;
    
    }
    
    ent->has_target = has_target_now;

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
                rm = MatrixMultiply(rm, ent->body_matrix);
                ent->body_matrix = rm;
            }
            break;
        case ENT_STATE_SEARCHING_TARGET:
            {
                ent->forward_angle += gst->dt;
                Matrix rm = MatrixRotateY(ent->forward_angle);
                rm = MatrixMultiply(rm, ent->body_matrix);
                ent->body_matrix = rm;
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
                ent->body_matrix = MatrixMultiply(qm, ent->body_matrix);
      
            }
            break;

        case ENT_STATE_HAS_TARGET:
            {
                ent->forward_angle = angle_xz(gst->player.position, ent->position);
                Matrix rm = MatrixRotateY(ent->forward_angle);
                rm = MatrixMultiply(rm, ent->body_matrix);
                ent->body_matrix = rm;
    
                Vector3 prj_position = ent->position;


                float angle = ent->forward_angle;
                const float ang_rad = 1.5;


                /*
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
                */
            }
            break;
    }
}

void enemy_lvl0_render(struct state_t* gst, struct enemy_t* ent) {
    if(ent->alive) {
        // Turret body
        DrawMesh(ent->model.meshes[0],
                 ent->model.materials[0],
                 ent->body_matrix);
        // Turret legs
        DrawMesh(ent->model.meshes[1],
                 ent->model.materials[0],
                 ent->model.transform);
    }
    else 
    if(!ent->broken_model_despawned 
    && IsModelValid(ent->broken_model)
    && ent->broken_matrices) {
        for(int i = 0; i < ent->broken_model.meshCount; i++) {
            DrawMesh(ent->broken_model.meshes[i], ent->model.materials[0], ent->broken_matrices[i]);
        }
    }
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

