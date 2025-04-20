#include <raylib.h>
#include <raymath.h>
#include <stdio.h>

#include "state/state.h"
#include "player.h"
#include "util.h"

#include "particle_systems/weapon_psys.h"
#include "projectile_mod/prjmod_test.h"

#define NOCLIP_SPEED 25

static void set_player_default_stats(struct player_t* p) {

    // Movement
    p->walkspeed = 20.0;
    p->run_speed_mult = 2.32;
    p->air_speed_mult = 1.5;
    p->jump_force = 130.0;
    p->gravity = 0.5;
    p->ground_friction = 0.015;
    p->air_friction = 0.001;
    p->dash_speed = 400.0;
    p->dash_timer_max = 4.0;

    // Armor and health

    p->max_health = MAX_DEFAULT_HEALTH;
    p->health = p->max_health;

    p->max_armor = MAX_DEFAULT_ARMOR;
    p->armor = p->max_armor;

    // Weapon stuff
    
    p->accuracy_modifier = 0.0;
    p->recoil_control = 0.0;
    p->firerate = 0.1;

    for(size_t i = 0; i < MAX_POWERUP_TYPES; i++) {
        p->powerup_levels[i] = 0.0;
    }

    // Misc

    // (skip first because it is the gun.)
    for(size_t i = 1; i < INV_SIZE; i++) {
        p->inventory.items[i] = NULL;
    }

    for(size_t i = 0; i < MAX_ENEMY_TYPES; i++) {
        p->kills[i] = 0;
    }

    p->xp = 999999;
}

void init_player_struct(struct state_t* gst, struct player_t* p) {
    p->render = 1; 


    // Figure out spawn point by looping through all chunks and taking highest point.
    p->spawn_point = (Vector3){ 0, 0, 0 };
    for(size_t i = 0; i < gst->terrain.num_chunks; i++) {
        struct chunk_t* chunk = &gst->terrain.chunks[i];
        if(chunk->center_pos.y > p->spawn_point.y) {
            p->spawn_point = chunk->center_pos;
        }
    }
    
    p->cam = (Camera){ 0 };
    p->cam.position = p->spawn_point;
    p->cam.target = (Vector3){ 0, 0, 0 };
    p->cam.up = (Vector3){ 0.0, 1.0, 0.0 };
    p->cam.fovy = 60.0;
    p->cam.projection = CAMERA_PERSPECTIVE;
    
    /*
    gst->shadow_cam = p->cam;
    gst->shadow_cam.fovy = 60.0;
    
    CameraPitch(&gst->shadow_cam, -90*DEG2RAD, 1, 0, 0);
    gst->shadow_cam.projection = CAMERA_ORTHOGRAPHIC;//PERSPECTIVE;    
    gst->shadow_cam_y = 800;
    */

    printf("Spawn point: %0.2f, %0.2f, %0.2f\n", 
            p->cam.position.x, p->cam.position.y, p->cam.position.z);

    p->position = (Vector3) { 0.0, 0.0, 0.0 };
    p->height = 30.5;
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
    p->fovy_change = 0.0;
    p->item_in_crosshair = NULL;
    p->speed = 0.0;
    p->onground = 1;
    p->num_jumps_inair = 0;

    p->gun_draw_timer = 0.0;
    p->gun_draw_speed = 8.3;

    p->recoil_timer = 0.0;
    p->recoil = 0.0;
    p->recoil_strength = 0.0;
    p->recoil_in_progress = 0;

    p->alive = 1;
    p->time_from_last_shot = 0.0;
    p->firerate_timer = 0.0;
    p->disable_aim_mode = DISABLE_AIM_WHEN_MOUSERIGHT;
    p->powerup_shop.open = 0;
    p->powerup_shop.selected_index = -1;
    p->inventory.selected_index = 0;
    p->holding_gun = 1;

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
    p->gunfx_timer = 1.0;

    for(size_t i = 0; i < MAX_ENEMY_TYPES; i++) {
        p->kills[i] = 0;
    }

    for(size_t i = 0; i < MAX_PRJMOD_INDICES; i++) {
        p->prjmod_indices[i] = -1;
    }


    p->gun_item = (struct item_t) {
        .rarity = ITEM_SPECIAL,
        .type = -1,
        .consumable = 0,
        .can_be_dropped = 0,
        .inv_tex = LoadTexture("res/textures/gun_inv.png")
    };
    inv_add_item(gst, p, &p->gun_item);

    // Calculate matrices for when player is aiming and not aiming.
    
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

    set_player_default_stats(p);

    gst->init_flags |= INITFLG_PLAYER;
}

void delete_player(struct state_t* gst, struct player_t* p) {
    if(!(gst->init_flags & INITFLG_PLAYER)) {
        return;
    }
    UnloadModel(p->gunmodel);
    UnloadModel(p->gunfx_model);
    UnloadTexture(p->gun_item.inv_tex);
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
    const float movef = 10.0 + (p->weapon.prj_scale * 2.0);
    prj_position.x += p->looking_at.x * movef;
    prj_position.y += p->looking_at.y * movef;
    prj_position.z += p->looking_at.z * movef;

    add_projectile(gst, &gst->psystems[PLAYER_WEAPON_PSYS], &p->weapon, 
            prj_position, p->looking_at, p->accuracy_modifier);

    // Recoil animation.
    p->recoil = 0.2;
    p->recoil_done = 0;
    p->recoil_timer = 0.0;
    p->recoil_strength = 0.5;
    p->recoil_in_progress = 1;

    p->time_from_last_shot = 0.0;
    p->accuracy_modifier += 0.45 * (1.0-p->recoil_control);

    p->accuracy_modifier = CLAMP(p->accuracy_modifier, WEAPON_ACCURACY_MIN, WEAPON_ACCURACY_MAX);
    //_clamp_accuracy_modifier(&p->accuracy_modifier, p->accuracy_control);


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
    if(p->powerup_shop.open) {
        return;
    }
    if(damage < 0.0) {
        return;
    }

    if(p->armor > 0) {
        p->armor -= 1.0;
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
    gst->xp_update_timer = 0.0;
    gst->xp_update_target = gst->player.xp + xp;
    gst->xp_update_done = 0;

    int value_add = round(map(fabs((float)xp), 0, 10000, 1.0, 10.0));
    gst->xp_update_add = value_add;

    //printf("'%s': %i, add=%i, target=%i\n", __func__, xp, value_add, gst->xp_update_target);
}

void player_apply_force(struct state_t* gst, struct player_t* p, Vector3 force) {
    p->ext_force_acc.x += force.x;
    p->ext_force_acc.z += force.z;
    p->ext_force_acc.y = 0;  // Gravity is handled differently.
    p->ext_force_vel.y = 0;

    p->velocity.y = force.y * 5.0;

}

int point_in_player_view(struct state_t* gst, struct player_t* p, Vector3 point, float fov_range) {

    // Need to ignore Y axis
    Vector3 P1 = (Vector3) { point.x, 0, point.z };
    Vector3 P2 = (Vector3) { p->position.x, 0, p->position.z };

    //Vector3 up = GetCameraUp(&p->cam);
    //Vector3 right = GetCameraRight(&p->cam);
    //Vector3 forward = Vector3CrossProduct(up, right);

    Vector3 dir = Vector3Normalize(Vector3Subtract(P1, P2));
    float dot = Vector3DotProduct(dir, p->cam_forward);

    // Map result of dot product to 0 - 180
    float f = map(dot, 1.0, -1.0, 0.0, 180.0);

    return (f < fov_range);
}

int playerin_biomeshift_area(struct state_t* gst, struct player_t* p) {
    int inarea = -1;
    const float shiftarea = gst->terrain.biomeshift_area;
    const float py = p->position.y;
    const float biome_div = gst->terrain.highest_point / (float)MAX_BIOME_TYPES;
    for(int i = 0; i < MAX_BIOME_TYPES; i++) {
        Vector2 biome_ylevel = gst->terrain.biome_ylevels[i];
        
        if(py < (biome_ylevel.y+shiftarea) && py > (biome_ylevel.y)) {
            inarea = i;
            break;
        }

    }

    return inarea;
}


void player_respawn(struct state_t* gst, struct player_t* p) {
    if(p->alive) {
        return;
    }
    p->alive = 1;
    p->is_aiming = 0;
    p->ready_to_shoot = 0;
    p->cam.position = p->spawn_point;
    p->velocity = (Vector3){0, 0, 0};
    p->ext_force_vel = (Vector3){0, 0, 0};
    p->ext_force_acc = (Vector3){0, 0, 0};
   
    set_player_default_stats(p);


    for(size_t i = 0; i < gst->num_enemies; i++) {
        gst->enemies[i].alive = 0;
    }


    delete_prjmods(gst);
    setup_default_enemy_spawn_settings(gst);

    DisableCursor();
}


void player_update(struct state_t* gst, struct player_t* p) {

    p->holding_gun = (p->inventory.selected_index == 0);
    //rainbow_palette(sin(gst->time*0.75), &p->weapon.color.r, &p->weapon.color.g, &p->weapon.color.b);

    int fireprj_hold = (gst->gamepad.id < 0) 
                    ? IsMouseButtonDown(MOUSE_BUTTON_LEFT) 
                    : IsGamepadButtonDown(gst->gamepad.id, GAMEPAD_BUTTON_RIGHT_TRIGGER_1);

    // 'any_gui_open' is set from state/state.c 'state_update_frame()'
    if(!p->any_gui_open
       && p->alive
       && p->holding_gun
       && ((gst->player.weapon_firetype == PLAYER_WEAPON_FULLAUTO)
        ? fireprj_hold
        : (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)))) {

        player_shoot(gst, &gst->player);
    }

   
    // aim idle timer should not be used if controller is detected.
    if(gst->gamepad.id < 0 && (((p->aim_idle_timer > 2.75) && p->is_aiming) || (!p->holding_gun))) {
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
        p->accuracy_modifier = CLAMP(p->accuracy_modifier, WEAPON_ACCURACY_MIN, WEAPON_ACCURACY_MAX);
    
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

    /*
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
    */

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

#include <rlgl.h>

void render_player(struct state_t* gst, struct player_t* p) {

    if(!p->render) {
        return;
    }
    if(!p->alive) {
        return;
    }
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

    const size_t inv_selected_i = p->inventory.selected_index;

    if(p->holding_gun) {

        // TODO: This should be renamed!
        // Update Gun Light here because otherwise it will be one frame behind.
        {
            Vector3 lpos = (Vector3){ 0.1, -0.52, -0.75 };
            lpos = Vector3Transform(lpos, p->gunmodel.transform);
            //DrawSphere(lpos, 0.05, RED);

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

        // Ammo level.

        shader_setu_float(gst, 
            ENERGY_LIQUID_SHADER,
            U_ENERGY_CONTAINER_LEVEL,
            &p->weapon.lqmag.ammo_level);

        shader_setu_float(gst,
                ENERGY_LIQUID_SHADER,
                U_ENERGY_CONTAINER_CAPACITY,
                &p->weapon.lqmag.capacity);

        DrawMesh(
                p->gunmodel.meshes[3],
                gst->energy_liquid_material,
                p->gunmodel.transform
                );

        //render_weapon_lqmag(gst, &p->weapon, p->gunmodel.transform);
    }
    else 
    if(inv_selected_i < INV_SIZE) {
        struct item_t* item_in_hands = p->inventory.items[inv_selected_i];
        if(!item_in_hands) {
            return;
        }

        Matrix item_offset = MatrixTranslate(1.0, -0.75, -2.0);
        Matrix item_scale = MatrixScale(0.3, 0.3, 0.3);

        Matrix item_transf = MatrixMultiply(item_offset, rotate_m);
        item_transf = MatrixMultiply(item_scale, item_transf);

        DrawMesh(
                item_in_hands->modelptr->meshes[0],
                item_in_hands->modelptr->materials[0],
                item_transf
                );

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
    if((IsKeyDown(KEY_LEFT_SHIFT) 
                || (gst->gamepad.id >= 0 && IsGamepadButtonDown(gst->gamepad.id, GAMEPAD_BUTTON_LEFT_THUMB)))
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
        p->speed *= NOCLIP_SPEED;
        p->onground = 0;
    }

    if(gst->gamepad.id >= 0) {
        p->velocity.x += gst->gamepad.Lstick.x * (p->speed * gst->dt);
        p->velocity.z += (-gst->gamepad.Lstick.y) * (p->speed * gst->dt);
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
    

    //CameraMoveForward (&gst->shadow_cam, (p->speed * p->velocity.z) * gst->dt, 1);
    //CameraMoveRight   (&gst->shadow_cam, (p->speed * p->velocity.x) * gst->dt, 1);

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

    // "View bobbing"
    {
        float vlen = Vector3Length((Vector3){ p->velocity.x, 0, p->velocity.z });
        vlen *= vlen;
        vlen = CLAMP(vlen, 0.0, 8.0);

        float yaw = 0.0;
        float pitch = 0.0;

        if(vlen > 0.01) {
            yaw   = sin((gst->time) * vlen) * 0.000075;
            pitch = cos((gst->time*2.0) * vlen) * 0.000075;
            CameraYaw(&gst->player.cam, yaw, 0);
            CameraPitch(&gst->player.cam, pitch, 1, 0, 0);
        }
    }
    
    // Fix camera target. it may be wrong if Y position changed.
    {
        float scale_up = ( p->position.y - p->cam.position.y);

        Vector3 up = Vector3Scale(GetCameraUp(&p->cam), scale_up);
        p->cam.target = Vector3Add(p->cam.target, up);
        p->cam.position.y = p->position.y;
    }


    // Check if current biome has changed.
    int biomeid_by_y = get_biomeid_by_ylevel(gst, p->position.y);
    if(biomeid_by_y != p->current_biome->id) {
        change_to_biome(gst, biomeid_by_y);
    }
}

void player_update_camera(struct state_t* gst, struct player_t* p) {
    if(p->any_gui_open || gst->devmenu_open) {
        return;
    }

    Vector2 md = GetMouseDelta();
  
    if(gst->gamepad.id >= 0) {
        md = Vector2Scale(gst->gamepad.Rstick, gst->gamepad.sensetivity);
    }


    p->cam_yaw = (-md.x * CAMERA_SENSETIVITY);
    p->looking_at = Vector3Normalize(Vector3Subtract(gst->player.cam.target, gst->player.cam.position));
        
    CameraYaw(&gst->player.cam, (-md.x * CAMERA_SENSETIVITY), 0);
    CameraPitch(&gst->player.cam, (-md.y * CAMERA_SENSETIVITY), 1, 0, 0);
    
    Vector3 up = GetCameraUp(&p->cam);
    Vector3 right = GetCameraRight(&p->cam);
    Vector3 forward = Vector3CrossProduct(up, right);

    p->cam_forward = forward;

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
        return;
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
        const Vector2 xp_text_pos = (Vector2) { 50.0, gst->res_y - 50.0 };
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

