#include <raylib.h>
#include <raymath.h>
#include <stdio.h>

#include "state.h"
#include "player.h"
#include "util.h"


void init_player_struct(struct state_t* gst, struct player_t* p) {

    p->cam = (Camera){ 0 };
    p->cam.position = (Vector3){ 2.0, 3.0, 0.0 };
    p->cam.target = (Vector3){ -1.0, 3.0, 1.0 };
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

    p->gun_draw_timer = 0.0;
    p->gun_draw_speed = 4.3;


    // --- Setup Weapon ---
    {
        gst->player.weapon = (struct weapon_t) {
            .projectiles = (struct projectile_t) { 0 },
            .num_alive_projectiles = 0,
            .knockback = 2.0,
            .accuracy = 7.5,
            .prj_speed = 50,
            .prj_damage = 10,
            .prj_max_lifetime = 3.0,
            .prj_size = (Vector3) { 1.0, 1.0, 1.0 }
        };
    }



    setup_3Dmodel(gst, &p->gunmodel, "res/models/gun_v1.glb", GUN_0_TEXID, (Vector3){0});

    // calculate matrices for when player is aiming and not aiming.
    
    // (Not aiming)
    {
        p->gunmodel_rest_offset_m
            = MatrixTranslate(0.0, -0.8, -0.5);

        Matrix rotate = MatrixRotateXYZ((Vector3){ 0.0, 0.0, 0.0 });
        p->gunmodel_rest_offset_m = MatrixMultiply(p->gunmodel_rest_offset_m, rotate);
    }

    // (Aiming)
    {
        p->gunmodel_aim_offset_m 
            = MatrixTranslate(0.0, 0.0, 0.0);
    }

}

void free_player(struct player_t* p) {
    UnloadModel(p->gunmodel);
}

void player_shoot(struct state_t* gst, struct player_t* p) {

    if(!p->is_aiming) {
        return;
    }

    Vector3 prj_position = (Vector3){0};
    prj_position = Vector3Transform(prj_position, p->gunmodel.transform);

    // move the projectile little bit forward so its not inside of the model.
    prj_position.x += p->looking_at.x*2;
    prj_position.y += p->looking_at.y*2 - 0.01;
    prj_position.z += p->looking_at.z*2;

    weapon_add_projectile(
            gst,
            &p->weapon,
            prj_position,
            p->looking_at // projectile direction
            );

}


/*
void player_shoot(struct state_t* gst, struct player_t* p) {
   
    size_t index = p->gun.proj_nextindex;
    struct projectile_t* proj = &p->gun.projectiles[index];

    proj->alive = 1;
    proj->lifetime = 0.0;
    proj->direction = p->looking_at;

    proj->position = (Vector3) { 0 };
    proj->position = Vector3Transform(proj->position, p->gunmodel.transform);

    //proj->light_index = gst->next_projlight_index;

    p->gun.proj_nextindex++;
    if(p->gun.proj_nextindex >= WEAPON_MAX_PROJECTILES) {
        p->gun.proj_nextindex = 0;
    }

    proj->light_index = gst->next_projlight_index;
    enable_projectile_light(gst, proj->light_index, proj->position);
}

void kill_projectile(struct state_t* gst, struct projectile_t* proj) {
    proj->lifetime = 0.0;
    proj->alive = 0;
    disable_projectile_light(gst, proj->light_index);
}
*/

void update_projectiles(struct state_t* gst, struct player_t* p) {
 
    /*
    struct projectile_t* proj = NULL;
    for(size_t i = 0; i < WEAPON_MAX_PROJECTILES; i++) {
        proj = &p->gun.projectiles[i];
        if(!proj->alive) {
            continue;
        }

        proj->lifetime += gst->dt;
        if(proj->lifetime >= p->gun.proj_max_lifetime) {
            kill_projectile(gst, proj);
            continue;
        }

        add_movement_vec3(&proj->position, proj->direction, gst->dt * p->gun.proj_speed);
   

        Light* light = &gst->projectile_lights[proj->light_index];
        float lightpos[3] = { 
            proj->position.x,
            proj->position.y,
            proj->position.z
        };
    
        SetShaderValue(gst->shaders[DEFAULT_SHADER], light->positionLoc, lightpos, SHADER_UNIFORM_VEC3);
      
        */

        /*

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

                enemy_hit(gst, enemy, proj);
                kill_projectile(gst, proj);

                printf("\033[31m >> ENEMY HIT (%li) |  Health: %i\033[0m\n", enemy->id, enemy->health);
            }
        }
        */
    //}
}


void player_render(struct state_t* gst, struct player_t* p) {
        
    // Draw player holding a gun.


    Matrix rotate_m = MatrixInvert(GetCameraViewMatrix(&p->cam));
    Matrix transform = MatrixTranslate(0.0, 0.0, 0.0);

    Matrix offset = (p->is_aiming) ? p->gunmodel_aim_offset_m : p->gunmodel_rest_offset_m;


    Quaternion no_aim_q = QuaternionFromMatrix(p->gunmodel_rest_offset_m);
    Quaternion aim_q    = QuaternionFromMatrix(p->gunmodel_aim_offset_m);

    Quaternion Q = QuaternionLerp(no_aim_q, aim_q, p->gun_draw_timer);

    transform = QuaternionToMatrix(Q);
    transform = MatrixMultiply(transform, offset);
    transform = MatrixMultiply(transform, rotate_m);

    
       

    p->gunmodel.transform = transform;

    DrawMesh(
            p->gunmodel.meshes[0],
            p->gunmodel.materials[0],
            p->gunmodel.transform
            );


    struct projectile_t* proj = NULL;
    Shader* shader = &gst->shaders[PLAYER_PROJECTILE_SHADER];
    int effectspeed_uniloc = gst->fs_unilocs[PLAYER_PROJECTILE_EFFECTSPEED_FS_UNILOC];
    float time = 2.0;

    SetShaderValue(*shader, effectspeed_uniloc, &time, SHADER_UNIFORM_FLOAT);
}



void update_player(struct state_t* gst, struct player_t* p) {

    weapon_update_projectiles(gst, &p->weapon);


    if(p->is_aiming) {
        p->gun_draw_timer += p->gun_draw_speed * gst->dt;
        if(p->gun_draw_timer > 1.0) {
            p->gun_draw_timer = 1.0;
            p->ready_to_shoot = 1;
        }
    
        printf("%f\n", p->gun_draw_timer);
    }

}



