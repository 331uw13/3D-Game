#include <raylib.h>
#include <raymath.h>
#include <stdio.h>

#include "state.h"
#include "player.h"
#include "util.h"


// PROJECTILE PARTICLE UPDATE ---
static void _player_weapon_psystem_projectile_pupdate(
        struct state_t* gst,
        struct psystem_t* psys,
        struct particle_t* part
){
    struct weapon_t* weapon = (struct weapon_t*)psys->userptr;
    if(!weapon) {
        MISSING_PSYSUSERPTR;
        return;
    }


    Vector3 vel = Vector3Scale(part->velocity, gst->dt * weapon->prj_speed);
    part->position = Vector3Add(part->position, vel);

    Matrix position_m = MatrixTranslate(part->position.x, part->position.y, part->position.z);
    *part->transform = position_m;

    part->light.position = part->position;
    update_light_values(&part->light, gst->shaders[DEFAULT_SHADER]);
}

// PROJECTILE PARTICLE INITIALIZATION ---
static void _player_weapon_psystem_projectile_pinit(
        struct state_t* gst,
        struct psystem_t* psys, 
        struct particle_t* part,
        Vector3 origin,
        Vector3 velocity,
        void* extradata, int has_extradata
){

    struct weapon_t* weapon = (struct weapon_t*)psys->userptr;
    if(!weapon) {
        MISSING_PSYSUSERPTR;
        return;
    }



    part->velocity = velocity;
    part->position = origin;
    Matrix position_m = MatrixTranslate(part->position.x, part->position.y, part->position.z);


    add_projectile_light(gst, &part->light, part->position, weapon->prj_color, gst->shaders[DEFAULT_SHADER]);

    *part->transform = position_m;
    part->alive = 1;
    part->lifetime = 0.0;
    part->max_lifetime = weapon->prj_max_lifetime;
}





void init_player_struct(struct state_t* gst, struct player_t* p) {

    p->cam = (Camera){ 0 };
    p->cam.position = (Vector3){ 2.0, 3.0, 0.0 };
    p->cam.target = (Vector3){ -1.0, 3.0, 1.0 };
    p->cam.up = (Vector3){ 0.0, 1.0, 0.0 };
    p->cam.fovy = 60.0;
    p->cam.projection = CAMERA_PERSPECTIVE;

    p->position = (Vector3) { 0.0, 0.0, 0.0 };
    p->hitbox_size = (Vector3){ 1.0, 3.5, 1.0 };
    p->velocity = (Vector3){ 0.0, 0.0, 0.0 };
    p->walkspeed = 0.45;
    p->run_mult = 1.5;
    p->walkspeed_aim_mult = 0.5;
    p->jump_force = 0.128;
    p->gravity = 0.6;
    p->onground = 1;
    p->friction = 0.025;
    p->num_jumps_inair = 0;
    p->max_jumps = 2;

    p->gun_draw_timer = 0.0;
    p->gun_draw_speed = 6.3;

    p->recoil_timer = 0.0;
    p->recoil = 0.0;
    p->recoil_strength = 0.0;
    p->recoil_in_progress = 0;


    p->max_health = 500.0;
    p->health = p->max_health;


    setup_weapon(gst,
            &p->weapon,
            _player_weapon_psystem_projectile_pupdate,
            _player_weapon_psystem_projectile_pinit,
            (struct weapon_t)
            {
                .accuracy = 7.5,
                .prj_speed = 90,
                .prj_damage = 10,
                .prj_max_lifetime = 3.0,
                .prj_size = (Vector3){ 1.0, 1.0, 1.0 },
                .prj_color = (Color) { 20, 255, 255,  255 }
            }
            );

    p->firerate_timer = 0.0;
    p->firerate = 0.1;

    p->gunmodel = LoadModel("res/models/gun_v1.glb");
    p->gunmodel.materials[0].shader = gst->shaders[DEFAULT_SHADER];
    p->gunmodel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = gst->textures[GUN_0_TEXID];


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
            = MatrixTranslate(0.1, -0.1, -0.4);
    }

}

void free_player(struct player_t* p) {
    UnloadModel(p->gunmodel);
    delete_weapon(&p->weapon);
}

void player_shoot(struct state_t* gst, struct player_t* p) {

    if(!p->is_aiming) {
        return;
    }
    if(!p->ready_to_shoot) {
        return;
    }

    if(p->firerate_timer >= p->firerate) {
        Vector3 prj_position = (Vector3){0};
        prj_position = Vector3Transform(prj_position, p->gunmodel.transform);

        // move the projectile little bit forward so its not inside of the model.
        prj_position.x += p->looking_at.x*2;
        prj_position.y += p->looking_at.y*2 - 0.01;
        prj_position.z += p->looking_at.z*2;


        weapon_add_projectile(gst, &p->weapon, prj_position, p->looking_at);


        p->recoil = 0.2;
        p->recoil_done = 0;
        p->recoil_timer = 0.0;
        p->recoil_strength = 0.6;
        p->recoil_in_progress = 1;

        p->firerate_timer = 0.0;
    }
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

    DrawMesh(
            p->gunmodel.meshes[0],
            p->gunmodel.materials[0],
            p->gunmodel.transform
            );
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


    if(!p->noclip) {
        // Add movement to camera scaled with velocity.
        float vm = Vector3Length(p->velocity) * 0.01;
        p->cam.position.y += sin(gst->time*20.0)*vm;
        p->cam.position.x += cos(gst->time*10.0)*vm;
        p->cam.position.z += cos(gst->time*10.0)*vm;

    }
}



