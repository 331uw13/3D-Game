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
 
    // -- Movement stats -----
    
    p->walkspeed = 20.0;
    p->run_speed_mult = 1.85;
    p->air_speed_mult = 1.5;
    p->jump_force = 130.0;
    p->gravity = 0.5;
    p->ground_friction = 0.015;
    p->air_friction = 0.001;
    p->dash_speed = 400.0;
    p->dash_timer_max = 4.0;
    // ------------------------


    p->cam = (Camera){ 0 };
    p->cam.position = gst->terrain.valid_player_spawnpoint;
    p->cam.target = (Vector3){ 0, 0, 0 };
    p->cam.up = (Vector3){ 0.0, 1.0, 0.0 };
    p->cam.fovy = 60.0;
    p->cam.projection = CAMERA_PERSPECTIVE;

    printf("Spawn point: %0.2f, %0.2f, %0.2f\n", 
            p->cam.position.x, p->cam.position.y, p->cam.position.z);

    p->position = (Vector3) { 0.0, 0.0, 0.0 };
    p->height = 6.5;
    p->hitbox_size = (Vector3){ 1.5, 2.8, 1.5 };
    p->hitbox_y_offset = -1.0;
    p->velocity = (Vector3){ 0.0, 0.0, 0.0 };
    p->ext_force_vel = (Vector3){ 0.0, 0.0, 0.0 };
    p->ext_force_acc = (Vector3){ 0.0, 0.0, 0.0 };
   
    p->max_armor = MAX_DEFAULT_ARMOR;
    p->armor_damage_dampen = DEFAULT_ARMOR_DAMAGE_DAMPEN;
    p->armor = p->max_armor;

    p->in_water = 0;
    p->dash_timer = 0.0;
    p->enable_fov_effect = 1;
    p->kills = 0;
    p->fovy_change = 0.0;
    p->item_in_crosshair = NULL;
    p->speed = 0.0;
    p->onground = 1;
    p->num_jumps_inair = 0;
    p->max_jumps = 2;

    p->gun_draw_timer = 0.0;
    p->gun_draw_speed = 8.3;

    p->recoil_timer = 0.0;
    p->recoil = 0.0;
    p->recoil_strength = 0.0;
    p->recoil_in_progress = 0;

    p->max_health = 300.0;
    p->health = p->max_health;
    p->alive = 1;
    p->accuracy_modifier = 0.0;
    p->accuracy_control = 0.0;
    p->time_from_last_shot = 0.0;
    p->firerate = 0.065;
    p->firerate_timer = 0.0;
    p->disable_aim_mode = DISABLE_AIM_WHEN_MOUSERIGHT;
    p->inventory.open = 0;
    p->powerup_shop.open = 0;
    p->powerup_shop.selected_index = -1;
    p->powerup_shop.timeout_time = 0.0;

    p->aim_idle_timer = 0.0;
    p->weapon_firetype = PLAYER_WEAPON_FULLAUTO;
    p->rotation_from_hit = (Vector3){ 0, 0, 0 };
    
    p->gunmodel = LoadModel("res/models/gun_v1.glb");

    // Material for gun:
    p->gunmodel.materials[0].shader = gst->shaders[DEFAULT_SHADER];
    p->gunmodel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = gst->textures[METAL2_TEXID];

    // Material for arms + "fingers":
    p->arms_material = LoadMaterialDefault();
    p->arms_material.maps[MATERIAL_MAP_DIFFUSE].texture = gst->textures[PLAYER_SKIN_TEXID];
    p->arms_material.shader = gst->shaders[DEFAULT_SHADER];
 
    // Material for hands:
    p->hands_material = LoadMaterialDefault();
    p->hands_material.maps[MATERIAL_MAP_DIFFUSE].texture = gst->textures[PLAYER_HANDS_TEXID];
    p->hands_material.shader = gst->shaders[DEFAULT_SHADER];
       



    p->gunfx_model = LoadModelFromMesh(GenMeshPlane(1.0, 1.0, 1, 1));
    p->gunfx_model.materials[0] = LoadMaterialDefault();
    p->gunfx_model.materials[0].shader = gst->shaders[GUNFX_SHADER];
    p->gunfx_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = gst->textures[GUNFX_TEXID];
    p->gunfx_timer = 1.0;

    p->xp = 165;

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


    p->inventory.item_drag = NULL;
    for(size_t i = 0; i < INV_SIZE; i++) {
        p->inventory.items[i] = NULL;
    }

}

void delete_player(struct player_t* p) {
    UnloadModel(p->gunmodel);
    UnloadModel(p->gunfx_model);
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

    p->aim_idle_timer = 0.0;
    p->firerate_timer = 0.0;

    Vector3 prj_position = (Vector3){0};
    prj_position = Vector3Transform(prj_position, p->gunmodel.transform);

    // Move the projectile initial position little bit ahead.
    prj_position.x += p->looking_at.x * 10.0;
    prj_position.y += p->looking_at.y * 10.0;
    prj_position.z += p->looking_at.z * 10.0;

    add_projectile(gst, &gst->psystems[PLAYER_WEAPON_PSYS], &p->weapon, 
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

    p->gunfx_timer = 0.0;
}

void player_damage(struct state_t* gst, struct player_t* p, float damage) {
    if(!p->alive) {
        return;
    }
    if(p->noclip) {
        return;
    }
    if(damage < 0.0) {
        return;
    }

    if(p->armor > 0) {
        p->armor--;
        damage *= (1.0 - p->armor_damage_dampen);
    }

    p->health -= damage;

    if(p->health <= 0.001) {
        p->health = 0;
        p->alive = 0;
        EnableCursor();
    }

    const float r = 2.0;
    p->rotation_from_hit = (Vector3) {
        RSEEDRANDOMF(-r, r),
        RSEEDRANDOMF(-r*0.5, r*0.5),
        RSEEDRANDOMF(-r, r)
    };

    if(gst->has_audio) {
        SetSoundPitch(gst->sounds[PLAYER_HIT_SOUND], 1.0 - RSEEDRANDOMF(0.0, 0.35));
        PlaySound(gst->sounds[PLAYER_HIT_SOUND]);
    }   


    int num_particles = map(damage, 0.0, p->max_health, 15, 50);

    add_particles(gst,
            &gst->psystems[PLAYER_HIT_PSYS],
            num_particles,
            p->position,
            Vector3Normalize(Vector3Subtract(p->cam.target, p->cam.position)),
            (Color){ 65, 5, 2, 150 },
            NULL, NO_EXTRADATA, NO_IDB
            );
}


void player_heal(struct state_t* gst, struct player_t* p, float heal) {
    // TODO: effect.

    p->health += heal;
    p->health = CLAMP(p->health, 0.0, p->max_health);

}

void player_add_xp(struct state_t* gst, int xp) {
    gst->xp_value_add += xp;
    gst->xp_update_timer = 0.0;
}

void player_apply_force(struct state_t* gst, struct player_t* p, Vector3 force) {
    p->ext_force_acc.x += force.x;
    p->ext_force_acc.z += force.z;
    p->ext_force_acc.y = 0;  // Gravity is handled differently.
    p->ext_force_vel.y = 0;

    p->velocity.y = force.y * 5.0;

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
    p->is_aiming = 0;
    p->ready_to_shoot = 0;
    p->cam.position = gst->terrain.valid_player_spawnpoint;
    p->velocity = (Vector3){0, 0, 0};
    p->ext_force_vel = (Vector3){0, 0, 0};
    p->ext_force_acc = (Vector3){0, 0, 0};
    p->xp = 0;
    p->kills = 0;
   
    for(size_t i = 0; i < gst->num_enemies; i++) {
        gst->enemies[i].alive = 0;
    }

    setup_default_enemy_spawn_settings(gst);

    DisableCursor();
}

static float test = 0.0;

void player_update(struct state_t* gst, struct player_t* p) {


    //rainbow_palette(sin(gst->time), &p->weapon.color.r, &p->weapon.color.g, &p->weapon.color.b);

    if(!p->any_gui_open
       && p->alive
       && ((gst->player.weapon_firetype == PLAYER_WEAPON_FULLAUTO)
        ? (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        : (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))) {
        player_shoot(gst, &gst->player);
    }

    if((p->aim_idle_timer > 2.75) && p->is_aiming) {
        p->is_aiming = 0;
    }

    if(!p->is_aiming) {
        p->ready_to_shoot = 0;
        p->disable_aim_mode = DISABLE_AIM_WHEN_MOUSERIGHT;
    }

    if(p->is_aiming) {
        p->aim_idle_timer += gst->dt;
        p->gun_draw_timer += p->gun_draw_speed * gst->dt;
        if(p->gun_draw_timer > 1.0) {
            p->gun_draw_timer = 1.0;
            p->ready_to_shoot = 1;
        }
    }
    else
    if(p->gun_draw_timer > 0) {
        p->gun_draw_timer -= p->gun_draw_speed * gst->dt;
    }


    if(p->firerate_timer < p->firerate) {
        p->firerate_timer += gst->dt;
    }

    p->firerate_timer = CLAMP(p->firerate_timer, 0.0, p->firerate);
    p->time_from_last_shot += gst->dt;

    if(p->dash_timer < p->dash_timer_max) {
        p->dash_timer += gst->dt;
    }

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


    if((p->cam.position.y < gst->terrain.water_ylevel) && !p->in_water) {
        p->in_water = 1;
        gst->fog_density = 6.5;
        update_fog_settings(gst);
    }
    else
    if((p->cam.position.y > gst->terrain.water_ylevel) && p->in_water) {
        p->in_water = 0;
        gst->fog_density = 1.0;
        update_fog_settings(gst);
    }

    // FOV Effect. Change FOV smoothly based on player velocity.
    if(p->enable_fov_effect) {
        float vlen = Vector3Length((Vector3){ p->velocity.x, 0.0, p->velocity.z });
        vlen = CLAMP(vlen, 0, 5.0);
        
        if(vlen > 1.5) {
            p->fovy_change += gst->dt * 2.5;
        }
        else {
            p->fovy_change -= gst->dt * 1.2;
        }

        p->fovy_change = CLAMP(p->fovy_change, 0.0, 1.0);
        p->cam.fovy = map(p->fovy_change, 0.0, 1.0, 60.0, 66.0) + sin(gst->time)*0.85;
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


    if(p->alive) {
        // Update Gun Light here because otherwise it will be one frame behind.
        {
            Vector3 lpos = (Vector3){ 0.25, -0.125, -2.0 };
            lpos = Vector3Transform(lpos, p->gunmodel.transform);
            //DrawSphere(lpos, 0.2, RED);

            p->gun_light.position = lpos;
            p->gun_light.color = p->weapon.color;
            set_light(gst, &p->gun_light, LIGHTS_UBO);
        }


        // Gun
        DrawMesh(
                p->gunmodel.meshes[2],
                p->gunmodel.materials[0],
                p->gunmodel.transform
                );

        // Arms + "Fingers"
        DrawMesh(
                p->gunmodel.meshes[0],
                p->arms_material,
                p->gunmodel.transform
                );

        // Hands
        DrawMesh(
                p->gunmodel.meshes[1],
                p->hands_material,
                p->gunmodel.transform
                );
    }

    // Gun FX
    if(p->gunfx_timer < 1.0) {
        
        float color4f[4] = {
            (float)gst->player.weapon.color.r / 255.0,
            (float)gst->player.weapon.color.g / 255.0,
            (float)gst->player.weapon.color.b / 255.0,
            (float)gst->player.weapon.color.a / 255.0
        };

        SetShaderValue(gst->shaders[GUNFX_SHADER], 
                gst->fs_unilocs[GUNFX_SHADER_COLOR_FS_UNILOC], color4f, SHADER_UNIFORM_VEC4);

 
        p->gunfx_model.transform = p->gunmodel.transform;
        
        p->gunfx_model.transform 
            = MatrixMultiply(MatrixTranslate(0.28, -0.125, -3.5), p->gunfx_model.transform);
        p->gunfx_model.transform = MatrixMultiply(MatrixRotateX(1.5), p->gunfx_model.transform);

        float st = lerp(p->gunfx_timer, 2.0, 0.0);
        p->gunfx_model.transform = MatrixMultiply(MatrixScale(st, st, st), p->gunfx_model.transform);

        DrawMesh(
                p->gunfx_model.meshes[0],
                p->gunfx_model.materials[0],
                p->gunfx_model.transform
                );

        p->gunfx_timer += gst->dt*13.0;

    }
}


void player_update_movement(struct state_t* gst, struct player_t* p) {
    if(gst->menu_open) {
        return;
    }
    
    p->prev_position = p->position;


    p->ext_force_vel = Vector3Add(p->ext_force_vel, p->ext_force_acc);
    p->cam.position = Vector3Add(p->cam.position, Vector3Scale(p->ext_force_vel, gst->dt));
    p->cam.target   = Vector3Add(p->cam.target, Vector3Scale(p->ext_force_vel, gst->dt));
    
    const float ext_force_damp = pow(0.98, gst->dt * TARGET_FPS);
    p->ext_force_acc = Vector3Scale(p->ext_force_acc, ext_force_damp);
    p->ext_force_vel = Vector3Scale(p->ext_force_vel, ext_force_damp);


    if(p->velocity.y > 0.1) {
        p->onground = 0;
    }


    // Handle X,Z Movement.

    p->speed = p->walkspeed;
   
    // Can player run?
    if(IsKeyDown(KEY_LEFT_SHIFT)
    && p->onground
    && !p->any_gui_open
    && !p->is_aiming
    && !p->in_water
    ){
        p->speed *= p->run_speed_mult;
    }

    // Air speed multiplier.
    if(!p->onground && !p->noclip) {
        p->speed *= p->air_speed_mult;
    }

    // Decrease speed if any gui is open.
    if(p->any_gui_open) {
        p->speed *= 0.5;
    }

    // Decrease speed if player is in water.
    if(p->in_water) {
        p->speed *= 0.85;
    }

    // For noclip.
    if(p->noclip) {
        p->speed *= 20.0;
        p->onground = 0;
    }

    
    if(IsKeyDown(KEY_W)) {
        p->velocity.z += p->speed * gst->dt;
    }
    if(IsKeyDown(KEY_S)) {
        p->velocity.z -= p->speed * gst->dt;
    }
    if(IsKeyDown(KEY_D)) {
        p->velocity.x += p->speed * gst->dt;
    }
    if(IsKeyDown(KEY_A)) {
        p->velocity.x -= p->speed * gst->dt;
    }

    const float vmax = 3.0; // Max velocity.

    p->velocity.x = CLAMP(p->velocity.x, -vmax, vmax);
    p->velocity.z = CLAMP(p->velocity.z, -vmax, vmax);

    // Normal movement
    CameraMoveForward (&p->cam, (p->speed * p->velocity.z) * gst->dt, 1);
    CameraMoveRight   (&p->cam, (p->speed * p->velocity.x) * gst->dt, 1);

    // Dash movement
    CameraMoveForward (&p->cam, (p->dash_velocity.z) * gst->dt, 1);
    CameraMoveRight   (&p->cam, (p->dash_velocity.x) * gst->dt, 1);

    // Friction.
    float friction = (p->onground || p->noclip) ? p->ground_friction : p->air_friction;
    float f = pow(1.0 - friction, gst->dt * TARGET_FPS);
    p->velocity.x *= f;
    p->velocity.z *= f;

    p->position = p->cam.position;

    float dash_friction = pow(0.97, gst->dt * TARGET_FPS);
    p->dash_velocity.x *= dash_friction;
    p->dash_velocity.z *= dash_friction;



    // Handle Y Movement.

    if(!p->noclip) {

        // Can the player use dash ability?
        if(!p->onground 
            && IsKeyPressed(KEY_SPACE)
            && (p->dash_timer >= p->dash_timer_max)
            && !p->any_gui_open
            && !p->in_water) {
            p->dash_velocity.x = p->velocity.x * p->dash_speed;
            p->dash_velocity.z = p->velocity.z * p->dash_speed;
            p->dash_timer = 0.0;
        }

        // Can the player jump?
        if(IsKeyPressed(KEY_SPACE) 
                && ((p->onground && !p->any_gui_open) || p->in_water)) {
            p->velocity.y = (!p->in_water) ? p->jump_force : (p->jump_force*0.5);
            p->onground = 0;
        }


        RayCollision ray = raycast_terrain(&gst->terrain, p->position.x, p->position.z);
        const float heightvalue = ray.point.y + p->height;

        p->position.y += p->velocity.y * gst->dt;
        

        if((p->position.y < heightvalue) || p->onground) {
            p->position.y = heightvalue;
            p->velocity.y = 0;
            p->onground = 1;
        }
            
        // Apply gravity if player is not on the ground.
        if(!p->onground) {
            float g = (GRAVITY_CONST * (!p->in_water ? p->gravity : 0.1)) * gst->dt;
            p->velocity.y -= g;
        }




    }
    else {
        if(IsKeyDown(KEY_SPACE)) {
            p->position.y += 2*p->speed * gst->dt;
        }
        else 
        if(IsKeyDown(KEY_LEFT_CONTROL)) {
            p->position.y -= 2*p->speed * gst->dt;
        }
    }


    // Fix camera target. it may be wrong if Y position changed.

    float scale_up = ( p->position.y - p->cam.position.y);

    Vector3 up = Vector3Scale(GetCameraUp(&p->cam), scale_up);
    p->cam.target = Vector3Add(p->cam.target, up);
    p->cam.position.y = p->position.y;



}

void player_update_camera(struct state_t* gst, struct player_t* p) {
    if(p->any_gui_open) {
        return;
    }

    const Vector2 md = GetMouseDelta();
    
    p->cam_yaw = (-md.x * CAMERA_SENSETIVITY);
    p->looking_at = Vector3Normalize(Vector3Subtract(gst->player.cam.target, gst->player.cam.position));
        
    CameraYaw(&gst->player.cam, (-md.x * CAMERA_SENSETIVITY), 0);
    CameraPitch(&gst->player.cam, (-md.y * CAMERA_SENSETIVITY), 1, 0, 0);
}

#define NO_YINCREMENT 0
#define ADD_YINCREMENT 1

static void draw_stats_bar(
        struct state_t* gst,
        float  x,
        float* y, // Change y for next bar.
        const char* text,
        float bar_width,
        int y_increment_setting,
        float value, 
        float value_max,
        Color color
){

    const float font_size = 15.0;
    const float bar_height = 20.0;

    const Color bg_color = (Color){ 30, 30, 30, 180 };
    const Color ln_color = (Color){ 100, 100, 100, 255 };

    DrawRectangle(x-2, *y-2, bar_width+4, bar_height+4, bg_color);
    DrawRectangleLines(x-2, *y-2, bar_width+5, bar_height+5, ln_color);

    float value_to_width = map(value, 0.0, value_max, 0, bar_width);
    DrawRectangle(x, *y, value_to_width, bar_height, color);

    DrawTextEx(gst->font, text, (Vector2){x+10, *y+5 }, font_size, 
            FONT_SPACING, (Color){ 70, 80, 80, 250 });

    if(y_increment_setting) {
        *y += bar_height + 10.0;
    }
}

void render_player_stats(struct state_t* gst, struct player_t* p) {
  
    if(!p->alive) {
        gui_render_respawn_screen(gst);
    }


    float stats_x = 20.0;
    float stats_y = 20.0;

    draw_stats_bar(gst,
            stats_x, &stats_y, "Health", 200, NO_YINCREMENT,
            p->health, p->max_health, 
            color_lerp(normalize(p->health, 0.0, p->max_health),
                    PLAYER_HEALTH_COLOR_LOW, PLAYER_HEALTH_COLOR_HIGH));

    draw_stats_bar(gst,
            stats_x+220, &stats_y, "Armor", 130, ADD_YINCREMENT,
            p->armor, p->max_armor,
            (Color){ 30, 230, 230, 180 });

    draw_stats_bar(gst,
            stats_x, &stats_y, "Firerate", 200, NO_YINCREMENT,
            p->firerate_timer, p->firerate,
            (Color){ 230, 180, 50, 180 });

    draw_stats_bar(gst,
            stats_x+220, &stats_y, "Dash Ability", 200, ADD_YINCREMENT,
            p->dash_timer, p->dash_timer_max,
            (p->dash_timer >= p->dash_timer_max) 
            ? (Color){ 125 + 125*(0.5+0.5*sin(gst->time*3.0)), 90, 140, 255} 
            : (Color){ 230, 80, 130, 180 });

    draw_stats_bar(gst,
            stats_x, &stats_y, "Accuracy", 200, ADD_YINCREMENT,
            p->weapon.accuracy - p->accuracy_modifier,
            p->weapon.accuracy,
            (Color){ 30, 230, 250, 180 });



    // * Experience level text.
    // * Full/Semi auto text
    {
        const float off = 5.0;
        const Vector2 xp_text_pos = (Vector2) { 50.0, gst->scrn_h - 50.0 };
        const char* xp_text = TextFormat("XP: %i", p->xp);
        float font_size = 20.0;
        Vector2 measured = MeasureTextEx(gst->font, xp_text, font_size, FONT_SPACING);
        
        
        // XP
        DrawRectangleRounded(
                (Rectangle){
                    xp_text_pos.x-off, xp_text_pos.y-off,
                    measured.x+off*2, measured.y+off
                },
                0.5, 8,
                (Color){ 20, 20, 20, 200 }
                );
        DrawTextEx(
                gst->font,
                xp_text,
                xp_text_pos,
                font_size,
                FONT_SPACING,
                (Color){ 20, 200, 20, 255 }
                );

        
        // Full/Semi auto

        font_size = 15;
        const char* mode_text = (p->weapon_firetype == PLAYER_WEAPON_FULLAUTO) 
            ? "(x) Fullauto" : "(x) Semiauto";
        measured = MeasureTextEx(gst->font, mode_text, font_size, FONT_SPACING);
        const Vector2 mode_text_pos = (Vector2) { xp_text_pos.x, xp_text_pos.y - measured.y-15 };

        DrawRectangleRounded(
                (Rectangle){
                    mode_text_pos.x-off, mode_text_pos.y-off,
                    measured.x+off*2, measured.y+off
                },
                0.5, 8,
                (Color){ 20, 20, 20, 200 }
                );

        DrawTextEx(
                gst->font,
                mode_text,
                mode_text_pos,
                font_size,
                FONT_SPACING,
                (Color){ 20, 150, 20, 255 }
                );
    }



    // Show extra info about powerups if inventory is open.
    
    if(!p->inventory.open) {
        return;
    }

    Vector2 textpos = (Vector2){ 30, 400 };
    const float textpos_y_inc = 17.5;
    DrawTextEx(
            gst->font,
            TextFormat("Accuracy: %0.1f / %0.1f", p->weapon.accuracy, WEAPON_ACCURACY_MAX),
            textpos,
            15,
            FONT_SPACING,
            (Color){ 200, 200, 200, 255 }
            );
    textpos.y += textpos_y_inc;

    DrawTextEx(
            gst->font,
            TextFormat("Firerate: %0.1f / %0.1f", p->firerate, FIRERATE_MIN),
            textpos,
            15,
            FONT_SPACING,
            (Color){ 200, 200, 200, 255 }
            );
    textpos.y += textpos_y_inc;

    DrawTextEx(
            gst->font,
            TextFormat("Max Health: %0.1f / %i", p->max_health, ABS_MAX_HEALTH),
            textpos,
            15,
            FONT_SPACING,
            (Color){ 200, 200, 200, 255 }
            );
    textpos.y += textpos_y_inc;

    DrawTextEx(
            gst->font,
            TextFormat("Max Armor: %i / %i", p->max_armor, ABS_MAX_ARMOR),
            textpos,
            15,
            FONT_SPACING,
            (Color){ 200, 200, 200, 255 }
            );
    textpos.y += textpos_y_inc;

    DrawTextEx(
            gst->font,
            TextFormat("Weapon damage: %0.1f / %0.1f", p->weapon.damage, WEAPON_DAMAGE_MAX),
            textpos,
            15,
            FONT_SPACING,
            (Color){ 200, 200, 200, 255 }
            );
    textpos.y += textpos_y_inc;

    DrawTextEx(
            gst->font,
            TextFormat("Armor damage dampen: %0.1f / %0.1f", p->armor_damage_dampen, ARMOR_DAMAGE_DAMPEN_MAX),
            textpos,
            15,
            FONT_SPACING,
            (Color){ 200, 200, 200, 255 }
            );
    textpos.y += textpos_y_inc;






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

