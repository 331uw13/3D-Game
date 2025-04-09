
#include "enemy_lvl1.h"
#include "../state/state.h"
#include "../util.h"

#include <raymath.h>
#include <stdio.h>

#include <stdlib.h>


#define HOVER_YLEVEL 100.0
#define NEW_RND_DEST_RADIUS 150

// Fight mode settings:
#define FIGHT_RADIUS_MIN 150.0 // How close enemy can get to the player?
#define FIGHT_RADIUS_MAX 200.0 // How far can enemy go away from player?



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

static void pick_travel_dest_in_fight(struct state_t* gst, struct enemy_t* ent) {
    // Pick random point around player in 'FIGHT_RADIUS' to move towards.


    int attemps = 0;
    int max_attemps = 100;

    

    while(1) {
        float ang = RSEEDRANDOMF(-M_PI, M_PI);
        ent->travel.dest.x = gst->player.position.x + FIGHT_RADIUS_MAX * cos(ang);
        ent->travel.dest.z = gst->player.position.z + FIGHT_RADIUS_MAX * sin(ang);

        ent->travel.dest.y = gst->player.position.y;

        if(Vector3Distance(ent->travel.dest, gst->player.position) > FIGHT_RADIUS_MIN) {
            break; // Accept new position.
        }

        attemps++;
        if(attemps >= max_attemps) {
            break;
        }
    }


   
    ent->travel.travelled = 0.0;
    ent->travel.start = ent->position;


    ent->travel.dest_reached = 0;
}

static void enemy_target_found(struct state_t* gst, struct enemy_t* ent) {
    printf("Target Found\n");

    ent->Q_prev = ent->Q_now;
    ent->angle_change = 0.0;
    ent->state = ENT_STATE_HAS_TARGET;
    ent->travel.dest_reached = 1;
}

static void enemy_target_lost(struct state_t* gst, struct enemy_t* ent) {
    printf("Target Lost\n");
   
    ent->travel.dest_reached = 1;
    ent->angle_change = 0.0;
    ent->state = ENT_STATE_SEARCHING_TARGET;
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

    enemy_has_target(gst, ent, &ent->matrix[ENEMY_LVL1_BODY_MI], enemy_target_found, enemy_target_lost);


    ent->position.x += ent->knockback_velocity.x;
    ent->position.y += ent->knockback_velocity.y;
    ent->position.z += ent->knockback_velocity.z;

    ent->knockback_velocity = Vector3Scale(ent->knockback_velocity, pow(0.99, TARGET_FPS * gst->dt));


    switch(ent->state) {
        case ENT_STATE_HAS_TARGET:
            {
                // Rotate towards player smoothly.
                Matrix target = MatrixIdentity();
                ent->rotation = get_rotation_yz(ent->position, gst->player.position);
               
                target = MatrixMultiply(target, MatrixRotateZ(-ent->rotation.z));
                target = MatrixMultiply(target, MatrixRotateY(ent->rotation.y+M_PI));
                ent->Q_target = QuaternionFromMatrix(target);
                const float d = 0.35;

                ent->Q_now = QuaternionSlerp(ent->Q_prev, ent->Q_target, ent->angle_change/d);
                if(ent->angle_change < d) {
                    ent->angle_change += gst->dt;
                }

                if(ent->time_from_hit > 0.5) {
                    if(ent->travel.dest_reached) {
                        pick_travel_dest_in_fight(gst, ent);
                    }

                    if(ent->dist_to_player > FIGHT_RADIUS_MIN) {

                        float t = 0.5+0.5*sin((ent->travel.travelled) - (M_PI/2.0));
                        Vector3 tpos = Vector3Lerp(ent->travel.start, ent->travel.dest, t);

                        ent->position.x = tpos.x;
                        ent->position.z = tpos.z;



                        ent->travel.travelled += gst->dt * 0.535;
                        if(ent->travel.travelled >= 1.0) {
                            ent->travel.dest_reached = 1;
                        }
                    }
                }

                if(ent->firerate_timer >= ent->firerate && gst->player.alive) {
                    ent->firerate_timer = 0.0;
                    
                    float prj_xoff = cos(-ent->rotation.y + M_PI/2) * 8.5;
                    float prj_zoff = sin(-ent->rotation.y + M_PI/2) * 8.5;

                    Vector3 prj_pos = (Vector3) {
                        ent->position.x + (ent->gun_index ? prj_xoff : -prj_xoff),
                        ent->position.y,
                        ent->position.z + (ent->gun_index ? prj_zoff : -prj_zoff),
                    };

                    

                    // Try to predict player movement.
                    Vector3 player_vel
                        = Vector3Subtract(gst->player.position, gst->player.prev_position);

                    float prj_time = ent->dist_to_player / (gst->enemy_weapons[ENEMY_LVL1].prj_speed * gst->dt);
                    player_vel.y *= 0.5;
                    
                    Vector3 player_future_pos = (Vector3) {
                        gst->player.position.x + (player_vel.x * prj_time) + RSEEDRANDOMF(-2.0, 2.0),
                        gst->player.position.y + (player_vel.y * prj_time),
                        gst->player.position.z + (player_vel.z * prj_time) + RSEEDRANDOMF(-2.0, 2.0)
                    };

                    Vector3 prj_dir = Vector3Normalize(Vector3Subtract(player_future_pos, prj_pos));


                    // Move the projectile little bit forward.
                    const float ft = 10.0;
                    prj_pos.x += prj_dir.x * ft;
                    prj_pos.y += prj_dir.y * ft;
                    prj_pos.z += prj_dir.z * ft;

                    ent->gun_index = !ent->gun_index;

                    add_projectile(gst,
                            &gst->psystems[ENEMY_WEAPON_PSYS],
                            &gst->enemy_weapons[ENEMY_LVL1_WEAPON],
                            prj_pos, prj_dir, NO_ACCURACY_MOD);

                    // Add gunfx
                    add_particles(gst,
                            &gst->psystems[ENEMY_GUNFX_PSYS],
                            1,
                            prj_pos, (Vector3){0}, ent->weaponptr->color,
                            ent, HAS_EXTRADATA, NO_IDB);
                
                    if(gst->has_audio) {
                        SetSoundVolume(gst->sounds[ENEMY_GUN_SOUND], get_volume_dist(gst->player.position, ent->position));
                        PlaySound(gst->sounds[ENEMY_GUN_SOUND]);
                    }
                }

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
        Vector3 hit_position, Vector3 hit_direction, float knockback) {
  
    hit_direction.x += RSEEDRANDOMF(-1.0, 1.0);
    hit_direction.z += RSEEDRANDOMF(-1.0, 1.0);
    ent->knockback_velocity = Vector3Scale(hit_direction, knockback);

    ent->travel.travelled = 0.0;
    ent->travel.dest_reached = 1;

    ent->time_from_hit = 0;
}

void enemy_lvl1_death(struct state_t* gst, struct enemy_t* ent) {
    printf("(INFO) '%s': Enemy %li Died\n", __func__, ent->index);
    
}

void enemy_lvl1_created(struct state_t* gst, struct enemy_t* ent) {
    ent->state = ENT_STATE_SEARCHING_TARGET;
    ent->Q_now = QuaternionFromMatrix(MatrixRotateY(0));
}

