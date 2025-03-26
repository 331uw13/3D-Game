
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

    // TODO: Rewrite this. it is too complicated for no reason...

    if(ent->state != ENT_STATE_CHANGING_ANGLE) {
        set_body_transform(ent, ray.normal);
    }
   

    int has_target_now = enemy_can_see_player(gst, ent);
    int player_in_halfway = (ent->dist_to_player < (ent->target_range / 2.0));


    if(player_in_halfway) {
        ent->firerate = 0.075;
        ent->accuracy_modifier = 1.25;
    }
    else {
        ent->firerate = 0.2;
        ent->accuracy_modifier = 0.0;
    }


    // Enemy lvl0 ignores FOV because it is 180.0 degrees.
    if((ent->mood == ENT_HOSTILE) && (has_target_now && !ent->has_target)) {
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
        ent->state = ENT_STATE_SEARCHING_TARGET;
        
        ent->Q_rnd_target = QuaternionFromMatrix(ent->matrix[ENEMY_LVL0_BODY_MI]);

        ent->rnd_angle_change = 0.0;
        ent->has_target = 0;
    }

    ent->position.x += ent->knockback_velocity.x;
    ent->position.y += ent->knockback_velocity.y;
    ent->position.z += ent->knockback_velocity.z;
    ent->knockback_velocity = Vector3Scale(ent->knockback_velocity, pow(0.987, TARGET_FPS * gst->dt));

    switch(ent->state) {
        case ENT_STATE_HAS_TARGET:
            {
                ent->rotation = get_rotation_yz(ent->position, gst->player.position);

                if(ent->firerate_timer >= ent->firerate && gst->player.alive) {

                    float prj_xoff = cos(-ent->rotation.y + M_PI/2) * 8.0;
                    float prj_zoff = sin(-ent->rotation.y + M_PI/2) * 8.0;

                    if(!ent->gun_index) {
                        prj_xoff = -prj_xoff;
                        prj_zoff = -prj_zoff;
                    }
                    

                    Vector3 prj_pos = (Vector3) {
                        ent->position.x + prj_xoff + ray.normal.x * 8,
                        ent->position.y + 9.0,
                        ent->position.z + prj_zoff + ray.normal.z * 8
                    };

                    Vector3 player_pos = gst->player.position;

                    if(player_in_halfway) {
                        // Try to predict players movement.

                        Vector3 player_vel = 
                            Vector3Subtract(gst->player.position, gst->player.prev_position);

                        // Projectile travel time to target.
                        float prj_time = ent->dist_to_player / (gst->enemy_weapons[ENEMY_LVL0].prj_speed * gst->dt); 

                        player_vel.y *= 0.7;
                        player_pos.x = player_pos.x + (player_vel.x * prj_time);
                        //player_pos.y = player_pos.y + (player_vel.y * prj_time);
                        player_pos.z = player_pos.z + (player_vel.z * prj_time);
                    }
                    Vector3 prj_dir = Vector3Normalize(Vector3Subtract(player_pos, prj_pos));

                    // Move the projectile little bit forward.
                    const float ft = 10.0;
                    prj_pos.x += prj_dir.x * ft;
                    prj_pos.y += prj_dir.y * ft;
                    prj_pos.z += prj_dir.z * ft;

                    add_projectile(gst, 
                            &gst->psystems[ENEMY_WEAPON_PSYS],
                            &gst->enemy_weapons[ENEMY_LVL0_WEAPON], 
                            prj_pos, prj_dir, ent->accuracy_modifier);

                    ent->firerate_timer = 0.0;
                    ent->gun_index = !ent->gun_index;


                    // Add gunfx
                    add_particles(gst,
                            &gst->psystems[ENEMY_GUNFX_PSYS],
                            1,
                            prj_pos, (Vector3){0}, ent->weaponptr->color,
                            ent, HAS_EXTRADATA, NO_IDB
                            );
                    
                    if(gst->has_audio) {
                        SetSoundVolume(gst->sounds[ENEMY_GUN_SOUND], get_volume_dist(gst->player.position, ent->position));
                        PlaySound(gst->sounds[ENEMY_GUN_SOUND]);
                    }
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
        Vector3 hit_position, Vector3 hit_direction, float knockback) {
  

    ent->knockback_velocity = Vector3Scale(hit_direction, knockback*0.2);

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

