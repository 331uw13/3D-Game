#include <raylib.h>
#include <raymath.h>
#include <stdio.h>

#include "state.h"
#include "player.h"
#include "util.h"


void init_player_struct(struct state_t* gst, struct player_t* p) {

    p->cam = (Camera){ 0 };
    p->cam.position = (Vector3){ 0.0, 3.0, 0.0 };
    p->cam.target = (Vector3){ 0.0, 0.0, 1.0 };
    p->cam.up = (Vector3){ 0.0, 1.0, 0.0 };
    p->cam.fovy = 60.0;
    p->cam.projection = CAMERA_PERSPECTIVE;

    p->position = (Vector3) { 0.0, 0.0, 0.0 };
    //p->hitbox = (Vector3){ 1.0, 2.0, 1.0 };
    p->velocity = (Vector3){ 0.0, 0.0, 0.0 };
    p->walkspeed = 0.8;
    p->jump_force = 0.128;
    p->gravity = 0.6;
    p->run_mult = 2.3;
    p->onground = 1;
    p->friction = 0.075;
    p->num_jumps_inair = 0;
    p->max_jumps = 2;

    setup_3Dmodel(gst, &p->gun_model, "res/models/gun_0.glb", GUN_0_TEXID, (Vector3){0});
    p->gun_model_poffset = (Vector3){ 0.9, -0.5, -1.25 };


    setup_weapon(
            &p->gun,
            45.5, /* projectile speed */
            10.0, /* projectile damage */
            0.2,  /* knockback */
            2.0,  /* projectile max lifetime */
            (Vector3){ 0.2, 0.2, 0.2 } /* hitbox size */
            );

}

void free_player(struct player_t* p) {
    UnloadModel(p->gun_model);

}


void player_shoot(struct state_t* gst, struct player_t* p) {
   

    size_t index = p->gun.proj_nextindex;
    struct projectile_t* proj = &p->gun.projectiles[index];

    proj->alive = 1;
    proj->lifetime = 0.0;
    proj->direction = p->looking_at;

    proj->position = Vector3Transform(proj->position, p->gun_model.transform);


    p->gun.proj_nextindex++;
    if(p->gun.proj_nextindex >= WEAPON_MAX_PROJECTILES) {
        p->gun.proj_nextindex = 0;
    }

}



void update_projectiles(struct state_t* gst, struct player_t* p) {
   
    struct projectile_t* proj = NULL;
    for(size_t i = 0; i < WEAPON_MAX_PROJECTILES; i++) {
        proj = &p->gun.projectiles[i];
        if(!proj->alive) {
            continue;
        }

        proj->lifetime += gst->dt;
        if(proj->lifetime >= p->gun.proj_max_lifetime) {
            proj->lifetime = 0.0;
            proj->alive = 0;
        }

        add_movement_vec3(&proj->position, proj->direction, gst->dt * p->gun.proj_speed);
   

        BoundingBox proj_boundingbox
            = (BoundingBox)
        {
            (Vector3) { 
                proj->position.x - proj->hitbox.x/2,
                proj->position.y - proj->hitbox.y/2,
                proj->position.z - proj->hitbox.z/2
            },
            (Vector3) { 
                proj->position.x + proj->hitbox.x/2,
                proj->position.y + proj->hitbox.y/2,
                proj->position.z + proj->hitbox.z/2
            }
        };


        for(size_t i = 0; i < gst->num_enemies; i++) {
            struct enemy_t* enemy = &gst->enemies[i];
            
            BoundingBox enemy_boundingbox 
                = (BoundingBox)
            {
                (Vector3) {
                    enemy->position.x - enemy->hitbox.x/2,
                    enemy->position.y - enemy->hitbox.y/2,
                    enemy->position.z - enemy->hitbox.z/2
                },
                (Vector3) {
                    enemy->position.x + enemy->hitbox.x/2,
                    enemy->position.y + enemy->hitbox.y/2,
                    enemy->position.z + enemy->hitbox.z/2
                }
            };

            if(CheckCollisionBoxes(enemy_boundingbox, proj_boundingbox)) {
                proj->alive = 0;

                enemy->health -= p->gun.proj_damage;
                enemy->knockback_velocity = Vector3Scale(proj->direction, p->gun.knockback);
                enemy->hit_direction = proj->direction;

                float random_r_x = (float)GetRandomValue(-5.0, 5.0) / 10.0;
                float random_r_z = (float)GetRandomValue(-5.0, 5.0) / 10.0;
                enemy->rotation_from_hit = (Vector3){ random_r_x, 0.0, random_r_z };

                printf("\033[31m>> ENEMY HIT (%li) |  Health: %i\033[0m\n", enemy->id, enemy->health);
            }

        }


    }



}


void player_render(struct state_t* gst, struct player_t* p) {
        
    // Draw player holding a gun.

    Matrix offset_m = MatrixTranslate(p->gun_model_poffset.x, p->gun_model_poffset.y, p->gun_model_poffset.z);
    Matrix rotate_m = MatrixInvert(GetCameraViewMatrix(&p->cam));
    p->gun_model.transform = MatrixMultiply(offset_m, rotate_m);
    DrawMesh(
            p->gun_model.meshes[0],
            p->gun_model.materials[0],
            p->gun_model.transform
            );


    struct projectile_t* proj = NULL;
    for(size_t i = 0; i < WEAPON_MAX_PROJECTILES; i++) {
        proj = &p->gun.projectiles[i];
        if(!proj->alive) {
            continue;
        }


        // TODO shader for projectiles.
        
        DrawSphere(proj->position, 0.2, RED); 
    }


}


void update_player(struct state_t* gst, struct player_t* p) {

    update_projectiles(gst, p);

}



