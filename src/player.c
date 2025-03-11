#include <raylib.h>
#include <raymath.h>
#include <stdio.h>

#include "state.h"
#include "player.h"
#include "util.h"

#include "particle_systems/weapon_psys.h"



static void _clamp_accuracy_modifier(float* acc_mod, float acc_control) {
    *acc_mod = CLAMP(*acc_mod, 0.0, WEAPON_ACCURACY_MAX - acc_control);
}

void init_player_struct(struct state_t* gst, struct player_t* p) {

    p->cam = (Camera){ 0 };
    p->cam.position = (Vector3){ 2.0, 0.0, 0.0 };
    p->cam.target = (Vector3){ 0, 0, 0 };
    p->cam.up = (Vector3){ 0.0, 1.0, 0.0 };
    p->cam.fovy = 60.0;
    p->cam.projection = CAMERA_PERSPECTIVE;

    p->position = (Vector3) { 0.0, 0.0, 0.0 };
    p->height = 5.0;
    p->hitbox_size = (Vector3){ 1.5, 2.8, 1.5 };
    p->hitbox_y_offset = -1.0;
    p->velocity = (Vector3){ 0.0, 0.0, 0.0 };
    
    p->walkspeed = 20.0;
    p->run_mult = 2.3;
    p->walkspeed_aim_mult = 0.5;
    p->air_speed_mult = 1.5;

    p->jump_force = 130.0;
    p->gravity = 0.5;
    
    p->onground = 1;
    p->friction = 0.015;
    p->num_jumps_inair = 0;
    p->max_jumps = 2;

    p->gun_draw_timer = 0.0;
    p->gun_draw_speed = 6.3;

    p->recoil_timer = 0.0;
    p->recoil = 0.0;
    p->recoil_strength = 0.0;
    p->recoil_in_progress = 0;

    p->max_health = 300.0;
    p->health = p->max_health;
    p->alive = 1;

    p->accuracy_modifier = 0.0;
    p->accuracy_control = 3.0;
    p->time_from_last_shot = 0.0;
    p->firerate = 0.05;
    p->firerate_timer = 0.0;

    p->weapon_firetype = PLAYER_WEAPON_FULLAUTO;

    p->rotation_from_hit = (Vector3){ 0, 0, 0 };
    
    p->gunmodel = LoadModel("res/models/gun_v1.glb");
    p->gunmodel.materials[0].shader = gst->shaders[DEFAULT_SHADER];
    p->gunmodel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = gst->textures[GUN_0_TEXID];


    p->arms_material = LoadMaterialDefault();
    p->arms_material.maps[MATERIAL_MAP_DIFFUSE].texture = gst->textures[PLAYER_ARMS_TEXID];
    p->arms_material.shader = gst->shaders[DEFAULT_SHADER];

    // calculate matrices for when player is aiming and not aiming.
    
    // (Not aiming)
    {
        p->gunmodel_rest_offset_m
            = MatrixTranslate(0.3, -0.25, -0.9);

        Matrix rotate = MatrixRotateZYX((Vector3){ -0.1, 0.3, 0.05 });
        p->gunmodel_rest_offset_m = MatrixMultiply(p->gunmodel_rest_offset_m, rotate);
    }

    // (Aiming)
    {
        p->gunmodel_aim_offset_m 
            = MatrixTranslate(0.3, 0.3, -0.6);
    }

}

void delete_player(struct player_t* p) {
    UnloadModel(p->gunmodel);
    // ...
    
    printf("\033[35m -> Deleted Player\033[0m\n");
}

void player_shoot(struct state_t* gst, struct player_t* p) {

    if(!p->is_aiming) {
        return;
    }
    if(!p->ready_to_shoot) {
        return;
    }

    if(p->firerate_timer < p->firerate) {
        return;
    }


    p->firerate_timer = 0.0;

    Vector3 prj_position = (Vector3){0};
    prj_position = Vector3Transform(prj_position, p->gunmodel.transform);

    // Move the projectile initial position little bit ahead.
    prj_position.x += p->looking_at.x * 2.35;
    prj_position.y += p->looking_at.y * 2.35;
    prj_position.z += p->looking_at.z * 2.35;

    add_projectile(gst, &p->weapon_psys, &p->weapon, 
            prj_position, p->looking_at, p->accuracy_modifier);

    // Recoil animation.
    p->recoil = 0.2;
    p->recoil_done = 0;
    p->recoil_timer = 0.0;
    p->recoil_strength = 0.5;
    p->recoil_in_progress = 1;

    p->time_from_last_shot = 0.0;
    p->accuracy_modifier += 0.45;

    _clamp_accuracy_modifier(&p->accuracy_modifier, p->accuracy_control);


    p->gun_light.strength += 0.025;
    p->gun_light.strength = CLAMP(p->gun_light.strength, 0.35, 1.0);


    if(gst->has_audio) {
        PlaySound(gst->sounds[PLAYER_GUN_SOUND]);
    }
}


void player_hit(struct state_t* gst, struct player_t* p, struct weapon_t* weapon) {
    if(!p->alive) {
        return;
    }

    p->health -= get_weapon_damage(weapon, NULL);

    if(p->health <= 0.001) {
        p->alive = 0;
    }

    const float r = 5.0;
    p->rotation_from_hit = (Vector3) {
        RSEEDRANDOMF(-r, r),
        RSEEDRANDOMF(-r*0.5, r*0.5),
        RSEEDRANDOMF(-r, r)
    };

    if(gst->has_audio) {
        SetSoundPitch(gst->sounds[PLAYER_HIT_SOUND], 1.0 - RSEEDRANDOMF(0.0, 0.35));
        PlaySound(gst->sounds[PLAYER_HIT_SOUND]);
    }

    //printf("(PLAYER_HIT): \033[32mHealth: %0.1f\033[0m\n", p->health);
}


Vector3 Vec3Lerp(float t, Vector3 a, Vector3 b) {
    return (Vector3) {
        lerp(t, a.x, b.x),
        lerp(t, a.y, b.y),
        lerp(t, a.z, b.z)
    };
}

void player_respawn(struct state_t* gst, struct player_t* p) {
    if(p->alive) {
        return;
    }
    p->health = p->max_health;
    p->alive = 1;
}


void player_update(struct state_t* gst, struct player_t* p) {

    if(p->is_aiming) {
        p->gun_draw_timer += p->gun_draw_speed * gst->dt;
        if(p->gun_draw_timer > 1.0) {
            p->gun_draw_timer = 1.0;
            p->ready_to_shoot = 1;
        }
    }
    else if(p->gun_draw_timer > 0) {
        p->gun_draw_timer -= p->gun_draw_speed * gst->dt;
    }


    if(p->firerate_timer < p->firerate) {
        p->firerate_timer += gst->dt;
    }

    p->firerate_timer = CLAMP(p->firerate_timer, 0.0, p->firerate);
    p->time_from_last_shot += gst->dt;
   

    // Move accuracy modifier back to normal value.
    // TODO: "player->weapon_control" variable for this time?
    if(p->time_from_last_shot > 0.15) {
        p->accuracy_modifier -= gst->dt * 20.0;
        _clamp_accuracy_modifier(&p->accuracy_modifier, p->accuracy_control);
    
        p->gun_light.strength -= gst->dt*0.5;
        p->gun_light.strength = CLAMP(p->gun_light.strength, 0.35, 1.0);
    }

    p->gunmodel_aim_offset_m 
            = MatrixTranslate((0.1 + p->recoil*0.07), -0.1, (-0.4 + p->recoil));
   
    if(p->recoil_in_progress) {
        const float tf = 15.0;
        if(!p->recoil_done) {
            p->recoil_timer += gst->dt*tf;

            float i = (p->recoil_timer);
            i *= i;
            p->recoil = lerp(i, 0.0, p->recoil_strength);
            if(p->recoil_timer >= 1.0) {
                p->recoil_done = 1;
            }
        }

        if(p->recoil_done) {
            p->recoil_timer -= gst->dt*tf;
            p->recoil = lerp(p->recoil_timer, 0.0, p->recoil_strength);
            if(p->recoil_timer <= 0.0) {
                p->recoil_in_progress = 0;
            }
        }

    }

    p->is_moving 
         = !(FloatEquals(p->velocity.x, 0.0)
        && FloatEquals(p->velocity.y, 0.0)
        && FloatEquals(p->velocity.z, 0.0));



    float ratio = pow(0.28, gst->dt * 5.0);
  
    p->rotation_from_hit.x *= ratio;
    p->rotation_from_hit.y *= ratio;
    p->rotation_from_hit.z *= ratio;

    p->cam.target.x += p->rotation_from_hit.x * gst->dt;
    p->cam.target.y += p->rotation_from_hit.y * gst->dt;
    p->cam.target.x += p->rotation_from_hit.z * gst->dt;
}


void player_render(struct state_t* gst, struct player_t* p) {

    if(p->noclip) {
        return;
    }

    Matrix rotate_m = MatrixInvert(GetCameraViewMatrix(&p->cam));
    Matrix transform = MatrixTranslate(0.0, 0.0, 0.0);

    Matrix offset = (p->is_aiming) ? p->gunmodel_aim_offset_m : p->gunmodel_rest_offset_m;


    if(!p->is_aiming) {
        // some movement to look more natural.

        const float time = GetTime() * 3.0;
        Matrix move_anim = MatrixTranslate(
                0.0,
                sin(time)*0.015,
                sin(time)*0.018
                );
        
    
        offset = MatrixMultiply(move_anim, offset);
    }


    Quaternion no_aim_q = QuaternionFromMatrix(p->gunmodel_rest_offset_m);
    Quaternion aim_q    = QuaternionFromMatrix(p->gunmodel_aim_offset_m);

    Quaternion Q = QuaternionLerp(no_aim_q, aim_q, (p->gun_draw_timer * p->gun_draw_timer));

    transform = QuaternionToMatrix(Q);
    transform = MatrixMultiply(transform, offset);
    transform = MatrixMultiply(transform, rotate_m);


    p->gunmodel.transform = transform;


    if(p->alive) {
        // Update Gun Light here because otherwise it will be one frame behind.
        {
            Vector3 lpos = (Vector3){ 0.25, -0.125, -2.0 };

            lpos = Vector3Transform(lpos, p->gunmodel.transform);
            //DrawSphere(lpos, 0.2, RED);

            p->gun_light.position = lpos;
            p->gun_light.color = p->weapon.color;
            set_light(gst, &p->gun_light, gst->lights_ubo);
        }


        // Gun
        DrawMesh(
                p->gunmodel.meshes[1],
                p->gunmodel.materials[0],
                p->gunmodel.transform
                );

        // Hands
        DrawMesh(
                p->gunmodel.meshes[0],
                p->arms_material,
                p->gunmodel.transform
                );
    }

}



BoundingBox get_player_boundingbox(struct player_t* p) {
    return (BoundingBox) {
        (Vector3) { // Min box corner.
            (p->position.x) - p->hitbox_size.x/2,
            (p->position.y + p->hitbox_y_offset) - p->hitbox_size.y/2,
            (p->position.z) - p->hitbox_size.z/2
        },
        (Vector3) { // Max box corner.
            (p->position.x) + p->hitbox_size.x/2,
            (p->position.y + p->hitbox_y_offset) + p->hitbox_size.y/2,
            (p->position.z) + p->hitbox_size.z/2
        }
    };
}

