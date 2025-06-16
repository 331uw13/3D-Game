#include <raylib.h>
#include <raymath.h>
#include <stdio.h>

#include "state/state.h"
#include "player.h"
#include "util.h"

#include "particle_systems/weapon_psys.h"

#define NOCLIP_SPEED  20

static void set_player_default_stats(struct state_t* gst, struct player_t* p) {

    // Movement

    p->walkspeed = 20.0;
    p->run_speed_mult = 2.32;
    p->air_speed_mult = 1.5;
    p->jump_force = 90.0;
    p->gravity = 0.5;
    p->ground_friction = 0.015;
    p->air_friction = 0.001;
    p->dash_speed = 400.0;
    p->dash_timer_max = 4.0;
    p->cam_random_dir = (Vector2){ 0 };
    //p->movement_state = MOVEMENT_STATE_STANDING;
    // Armor and health

    p->max_health = MAX_DEFAULT_HEALTH;
    p->health = p->max_health;

    p->max_armor = MAX_DEFAULT_ARMOR;
    p->armor = p->max_armor;
    p->alive = 1;
    p->is_aiming = 0;

    // Misc

    for(size_t i = 0; i < MAX_ENEMY_TYPES; i++) {
        p->kills[i] = 0;
    }

    inventory_init(&p->inventory);

    p->can_interact = 0;
    p->interact_action = 0;
    p->weapon_offset_interp = 0.0;
    p->weapon_inspect_interp = 0.0;
    p->accuracy_modifier = 0.0;        

    p->changing_item = 0;
    p->item_change_timer = 0.0;
    p->item_to_change = NULL;
    
    p->item_in_hands = NULL;//&p->inventory.items[0];

    p->xp = 999999;
    p->chunk = NULL;

    for(int i = 0; i < MAX_HITBOXES; i++) {
        p->hitboxes[i].hits = 0;
    }

}

static void create_player_hitboxes(struct state_t* gst, struct player_t* p) {
    
    p->ccheck_radius = 60.0;


    const float head_size = 2.0;

    const float body_width = 3.0;
    const float legs_width = 2.0;

    const float body_height = (p->height/2.0) - head_size;


    p->hitboxes[HITBOX_HEAD] = (struct hitbox_t) {
        .size = (Vector3){ head_size, head_size, head_size },
        .offset = (Vector3){ 0, 0, 0 },
        .damage_mult = 3.0,
        .tag = HITBOX_HEAD
    };

    p->hitboxes[HITBOX_BODY] = (struct hitbox_t) {
        .size = (Vector3){ body_width, body_height, body_width },
        .offset = (Vector3){ 0, -(head_size/2 + body_height/2), 0 },
        .damage_mult = 1.0,
        .tag = HITBOX_BODY
    };

    p->hitboxes[HITBOX_LEGS] = (struct hitbox_t) {
        .size = (Vector3){ legs_width, body_height, legs_width },
        .offset = (Vector3){ 0, -(head_size/2 + body_height*1.5), 0 },
        .damage_mult = 0.75,
        .tag = HITBOX_LEGS
    };


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
    p->cam.fovy = 70.0;
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

    p->wants_to_pickup_item = 0;
    p->position = (Vector3) { 0.0, 0.0, 0.0 };
    p->height = 12.0;
    p->hitbox_size = (Vector3){ 1.5, 2.8, 1.5 };
    p->hitbox_y_offset = -1.0;
    p->velocity = (Vector3){ 0.0, 0.0, 0.0 };
    p->ext_force_vel = (Vector3){ 0.0, 0.0, 0.0 };
    p->ext_force_acc = (Vector3){ 0.0, 0.0, 0.0 };
   
    p->max_armor = MAX_DEFAULT_ARMOR;
    p->armor_damage_dampen = DEFAULT_ARMOR_DAMAGE_DAMPEN;
    p->armor = p->max_armor;

    p->dash_timer = 0.0;
    p->speed = 0.0;
    p->onground = 1;

    p->gunfx_model = LoadModelFromMesh(GenMeshPlane(1.0, 1.0, 1, 1));
    p->gunfx_model.materials[0] = LoadMaterialDefault();
    p->gunfx_model.materials[0].shader = gst->shaders[GUNFX_SHADER];
    p->gunfx_timer = 1.0;

    for(size_t i = 0; i < MAX_ENEMY_TYPES; i++) {
        p->kills[i] = 0;
    }

    set_player_default_stats(gst, p);
    gst->init_flags |= INITFLG_PLAYER;
    
    create_player_hitboxes(gst, p);
}

void delete_player(struct state_t* gst, struct player_t* p) {
    if(!(gst->init_flags & INITFLG_PLAYER)) {
        return;
    }

    UnloadModel(p->gunfx_model);

    // ... Maybe this function will be needed ?


    printf("\033[35m -> Deleted Player\033[0m\n");
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
        p->armor -= 1.0;
        damage *= (1.0 - p->armor_damage_dampen);
    }

    p->health -= damage;

    if(p->health <= 0.001) {
        p->health = 0;
        p->alive = 0;
        EnableCursor();
    }

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

    Vector3 dir = Vector3Normalize(Vector3Subtract(P1, P2));
    float dot = Vector3DotProduct(dir, p->cam_forward);

    // Map result of dot product to 0 - 180.
    // Its easier to use.
    float f = map(dot, 1.0, -1.0, 0.0, 180.0);

    return (f < fov_range);
}

int playerin_biomeshift_area(struct state_t* gst, struct player_t* p) {
    int inarea = -1;
    const float shiftarea = gst->terrain.biomeshift_area;
    const float py = p->position.y;
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
    p->cam.position = p->spawn_point;
    p->velocity = (Vector3){0, 0, 0};
    p->ext_force_vel = (Vector3){0, 0, 0};
    p->ext_force_acc = (Vector3){0, 0, 0};
   
    set_player_default_stats(gst, p);


    for(size_t i = 0; i < gst->num_enemies; i++) {
        gst->enemies[i].alive = 0;
    }


    setup_default_enemy_spawn_settings(gst);

    DisableCursor();
}


static void update_weapon_gravity(struct state_t* gst, struct player_t* p, struct weapon_model_t* weapon_model) {

    //weapon_model->gravity_offset.x = sin(gst->time);


    weapon_model->gravity_offset.x += p->cam_yaw;
    weapon_model->gravity_offset.y -= p->cam_pitch;

    float weapon_mass = 10.0;


    float dist = Vector3Distance(weapon_model->render_offset, weapon_model->gravity_offset);
    if(!FloatEquals(dist, 0.0)) {
        Vector3 direction = Vector3Subtract(weapon_model->render_offset, weapon_model->gravity_offset);
        float magnitude = weapon_mass / (dist * dist);
        magnitude = CLAMP(magnitude, -1.0, 1.0);

        direction = Vector3Scale(direction, magnitude);


        weapon_model->gravity_velocity 
            = Vector3Add(weapon_model->gravity_velocity, Vector3Scale(direction, gst->dt*GRAVITY_CONST));
       
        // Decrease velocity overtime.
        weapon_model->gravity_velocity
            = Vector3Scale(weapon_model->gravity_velocity,
                           pow(weapon_model->gravity_friction, gst->dt*GRAVITY_CONST));
       
        // Apply velocity to model offset.
        weapon_model->gravity_offset 
            = Vector3Add(weapon_model->gravity_offset,
                         Vector3Scale(weapon_model->gravity_velocity, gst->dt));
    

        weapon_model->gravity_offset.z = 0.0;
   }
}

static void update_weapon_in_hands(struct state_t* gst, struct player_t* p) {
    if(!p->item_in_hands) {
        return;
    }
    if(!p->item_in_hands->is_weapon_item) {
        return;
    }

    if(!p->any_gui_open
        && !p->inventory.open
        && p->alive 
        && (IsMouseButtonDown(MOUSE_BUTTON_LEFT))) {

        player_shoot(gst, &gst->player);
    }


    struct weapon_model_t* weapon_model = &p->item_in_hands->weapon_model;

    weapon_model->firerate_timer += gst->dt;
    update_weapon_gravity(gst, p, weapon_model);

    // Update weapon recoil animation.
    if(!weapon_model->recoil_anim_done) {
        if(weapon_model->recoil_anim_value < 1.0) {
            weapon_model->recoil_anim_value += gst->dt * 10.0;
        }
        else {
            weapon_model->recoil_anim_done = 1;
        }
    }
    else {
        if(weapon_model->recoil_anim_value > 0.0) {
            weapon_model->recoil_anim_value -= gst->dt * 10.0;
        }
    }



    // Update interpolation between aiming model offset and rest model offset.

    const float interp_speed = 5.0;
    if(p->is_aiming) {
        // Start to aim.
        if(p->weapon_offset_interp < 1.0) {
            p->weapon_offset_interp += gst->dt * (interp_speed * 2.0);
        }
    }
    else {
        // Stop aiming.
        if(p->weapon_offset_interp > 0.0) {
            p->weapon_offset_interp -= gst->dt * (interp_speed * 0.3);
        }
    }
    p->weapon_offset_interp = CLAMP(p->weapon_offset_interp, 0.0, 1.0);


    // Inspecting model interpolation.

    if(p->inspecting_weapon) {
        // Start to inspect weapon.
        if(p->weapon_inspect_interp < 1.0) {
            p->weapon_inspect_interp += gst->dt * 3.0;
        }
    }
    else {
        // Stop inspecting weapon.
        if(p->weapon_inspect_interp > 0.0) {
            p->weapon_inspect_interp -= gst->dt * 3.0;
        }
    }
    p->weapon_inspect_interp = CLAMP(p->weapon_inspect_interp, 0.0, 1.0);



    // Update gravity friction for weapon model.
    // This is because it looks kind of weird to have "over shoot" when starting to aim.
    int aiming_in_progress = (p->is_aiming && !FloatEquals(p->weapon_offset_interp, 1.0));
   
    // The gravity friction cannot be changed instantly
    // because the weapon model still has the same momentum.
    if(aiming_in_progress) {
        weapon_model->gravity_friction -= gst->dt;
    }
    else {
        weapon_model->gravity_friction += gst->dt*0.075;
    }

    weapon_model->gravity_friction 
        = CLAMP(weapon_model->gravity_friction,
                WMODEL_HIGH_GRAVITY_FRICTION,
                WMODEL_LOW_GRAVITY_FRICTION);

}

void player_update(struct state_t* gst, struct player_t* p) {

    p->chunk = find_chunk(gst, p->position);

    // Update item changing animation.
    if(p->changing_item) {
        p->item_change_timer += gst->dt * 3;

        if(!p->item_to_change) {
            fprintf(stderr, "\033[31m(ERROR) '%s': Cant change to NULL item\033[0m\n",
                    __func__);
            p->changing_item = 0;
        }

        // Change the item when item model is out of view.
        if(p->item_change_timer > 0.5
        && (p->item_in_hands != p->item_to_change)) {
            p->item_in_hands = p->item_to_change;
        }
        

        if(p->item_change_timer >= 1.0) {
            p->changing_item = 0;
        }
    }

    update_weapon_in_hands(gst, p);
}

void player_shoot(struct state_t* gst, struct player_t* p) {
    if(!p->is_aiming) {
        return;
    }
    if(!p->item_in_hands) {
        return;
    }
    if(!p->item_in_hands->is_weapon_item) {
        return;
    }
    if(p->weapon_inspect_interp > 0.01) {
        return;
    }
    if(p->any_gui_open || gst->devmenu_open) {
        return;
    }


    struct weapon_model_t* weapon_model = &p->item_in_hands->weapon_model;

    if(weapon_model->firerate > weapon_model->firerate_timer) {
        return;
    }

    weapon_model->firerate_timer = 0.0;


    if(weapon_model->stats.lqmag.ammo_level <= 0.0) {
        if(gst->has_audio) {
            PlaySound(gst->sounds[PLAYER_GUN_NOAMMO_SOUND]);
        }

        // Out of ammo.
        return;
    }


    Vector3 prj_position = (Vector3) { 0 };
    prj_position = Vector3Transform(prj_position, p->last_weapon_matrix);

    float movef = 10.0 + (weapon_model->stats.prj_scale * 2.0);
    prj_position.x += p->looking_at.x * movef;
    prj_position.y += p->looking_at.y * movef;
    prj_position.z += p->looking_at.z * movef;

    prj_position.y -= weapon_model->aim_offset.y;

    add_projectile(
            gst,
            &gst->psystems[PLAYER_WEAPON_PSYS],
            &weapon_model->stats,
            prj_position,
            p->looking_at,
            p->accuracy_modifier
            );


    p->gunfx_timer = 0.0;

    weapon_model->recoil_anim_value = 0.0;
    weapon_model->recoil_anim_done = 0;


    if(gst->has_audio) {
        PlaySound(gst->sounds[PLAYER_GUN_SOUND]);
    }


    // Add recoil animation.

    const float recoil_half = weapon_model->recoil / 2.0;
    p->cam_random_dir.x += RSEEDRANDOMF(-recoil_half, recoil_half);
    p->cam_random_dir.y += RSEEDRANDOMF(recoil_half, weapon_model->recoil);
}


void render_player(struct state_t* gst, struct player_t* p) {
    if(!p->alive) {
        return;
    }

    if(!p->item_in_hands) {
        return;
    }

    if(p->item_in_hands->inv_index < 0) {
        return;
    }

    Matrix icam_matrix = MatrixInvert(GetCameraViewMatrix(&p->cam));
    Matrix transform = MatrixIdentity();


    float item_change_yoff = 0;
    if(p->changing_item) {
        item_change_yoff = sin(p->item_change_timer * M_PI) * 3.0;
    }


    if(p->item_in_hands->is_weapon_item) {
        struct weapon_model_t* weapon_model = &p->item_in_hands->weapon_model;



        Matrix rest_rotation = MatrixRotateXYZ(weapon_model->rest_rotation);
        float interp = inout_cubic(p->weapon_offset_interp);

        Vector3 rest_offset = weapon_model->rest_offset;
        Vector3 aim_offset = weapon_model->aim_offset;
   
        rest_offset.y -= item_change_yoff; 

        if(p->is_aiming) {
            aim_offset.z += inout_cubic(weapon_model->recoil_anim_value) * weapon_model->recoil_anim_kickback;
        }


        Matrix model_inspect = MatrixIdentity();

        Vector3 offset = Vector3Lerp(
                rest_offset,
                aim_offset,
                interp
                );

        Quaternion qrot = QuaternionSlerp(
                QuaternionFromMatrix(rest_rotation),
                QuaternionFromMatrix(model_inspect),
                interp
                );

        // Add offsets if player is inspecting the weapon.
        if(p->weapon_inspect_interp > 0.01) {
            offset = Vector3Lerp(
                    offset,
                    weapon_model->inspect_offset,
                    inout_cubic(p->weapon_inspect_interp)
                    );

            qrot = QuaternionSlerp(
                    qrot,
                    QuaternionFromMatrix(MatrixRotateXYZ(weapon_model->inspect_rotation)),
                    p->weapon_inspect_interp
                    );
        }

        weapon_model->render_offset = offset;
        
        // Adjust gravity offset so its at the right level again.
        Vector3 adjusted_goffset = weapon_model->gravity_offset;
        adjusted_goffset.y -= offset.y;
        offset = Vector3Add(offset, adjusted_goffset);


        Matrix rotation = QuaternionToMatrix(qrot);
        Matrix model_offset = MatrixTranslate(offset.x, offset.y, offset.z);
        model_offset = MatrixMultiply(rotation, model_offset);
        transform = MatrixMultiply(model_offset, icam_matrix);


        
        p->last_weapon_matrix = transform;
       
        if(!p->in_scope_view) {
            render_weapon_model(gst, weapon_model, transform);
        }
    }
    else {
        // The item is probably normal item.
        // This offset should work for all of them. (?)
        // TODO: (Item model config?)

        Matrix model_offset = MatrixTranslate(2.8333, -2.3333 - item_change_yoff, -5.5000);
        transform = MatrixMultiply(model_offset, icam_matrix);        
    

        render_item(gst, p->item_in_hands, transform);

    }

}

void player_change_holding_item(struct state_t* gst, struct player_t* p, struct item_t* item) {
    if(p->changing_item) {
        return;
    }

    p->is_aiming = 0;
    p->changing_item = 1;
    p->item_to_change = item;
    p->item_change_timer = 0.0;

    if(item->is_weapon_item) {
        struct weapon_model_t* weapon_model = &item->weapon_model;

        weapon_model->gravity_friction = WMODEL_LOW_GRAVITY_FRICTION;
        weapon_model->gravity_offset   = (Vector3){ 0, 0, 0 };
        weapon_model->gravity_velocity = (Vector3){ 0, 0, 0 };
        weapon_model->render_offset    = (Vector3){ 0, 0, 0 };
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
    && !IsKeyDown(KEY_LEFT_CONTROL)
    && p->onground
    && !p->any_gui_open
    ){
        p->speed *= p->run_speed_mult;
    }

    if(IsKeyDown(KEY_LEFT_CONTROL)) {
        p->speed *= 0.65;
    }

    // Air speed multiplier.
    if(!p->onground && !p->noclip) {
        p->speed *= p->air_speed_mult;
    }

    // Decrease speed if any gui is open.
    if(p->any_gui_open) {
        p->speed *= 0.5;
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
    

    // Friction.
    float friction = (p->onground || p->noclip) ? p->ground_friction : p->air_friction;
    float f = pow(1.0 - friction, gst->dt * TARGET_FPS);
    p->velocity.x *= f;
    p->velocity.z *= f;

    p->position = p->cam.position;

    float dash_friction = pow(0.985, gst->dt * TARGET_FPS);
    p->dash_velocity.x *= dash_friction;
    p->dash_velocity.z *= dash_friction;



    // Handle Y Movement.

    if(!p->noclip) {

        // Can the player use dash ability?
        if(!p->onground 
            && IsKeyPressed(KEY_SPACE)
            && (p->dash_timer >= p->dash_timer_max)
            && !p->any_gui_open) {
            p->dash_velocity.x = p->velocity.x * p->dash_speed;
            p->dash_velocity.z = p->velocity.z * p->dash_speed;
            p->dash_timer = 0.0;
        }

        // Can the player jump?
        if(IsKeyPressed(KEY_SPACE) && (p->onground && !p->any_gui_open)) {
            p->velocity.y = p->jump_force;
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
            float g = (GRAVITY_CONST * p->gravity) * gst->dt;
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


    // Accuracy Modifier.

    float velocity_len_max = p->run_speed_mult * 2.0;
    float velocity_len = Vector3Length(p->velocity);
    velocity_len = CLAMP(velocity_len, 0.0, velocity_len_max);
    p->accuracy_modifier = map(velocity_len, 0.0, velocity_len_max, 0.0, 4.0);
}

void player_update_camera(struct state_t* gst, struct player_t* p) {

    if(p->any_gui_open || (gst->devmenu_open && !IsMouseButtonDown(MOUSE_RIGHT_BUTTON))) {
        return;
    }


    Vector2 md = GetMouseDelta();
  
    if(gst->gamepad.id >= 0) {
        md = Vector2Scale(gst->gamepad.Rstick, gst->gamepad.sensetivity);
    }


    p->cam_yaw = (-md.x * CAMERA_SENSETIVITY);
    p->cam_pitch = (-md.y * CAMERA_SENSETIVITY);
    
    p->looking_at = Vector3Normalize(Vector3Subtract(gst->player.cam.target, gst->player.cam.position));


    CameraYaw(&gst->player.cam,   (-md.x * CAMERA_SENSETIVITY) + p->cam_random_dir.x, 0);
    CameraPitch(&gst->player.cam, (-md.y * CAMERA_SENSETIVITY) + p->cam_random_dir.y, 1, 0, 0);
    
    float angle_dampen = pow(0.95, gst->dt*500);
    angle_dampen = (angle_dampen >= 0.999) ? 0.95 : angle_dampen; 
    p->cam_random_dir.x *= angle_dampen;
    p->cam_random_dir.y *= angle_dampen;

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

void render_player_gunfx(struct state_t* gst, struct player_t* p) {
    if(!p->item_in_hands) {
        return;
    }
    if(!p->item_in_hands->is_weapon_item) {
        return;
    }
    if(p->in_scope_view) {
        return;
    }

    struct weapon_model_t* weapon_model = &p->item_in_hands->weapon_model;


    if(p->gunfx_timer < 1.0) {
        p->gunfx_model.transform = p->last_weapon_matrix;
       
        // Offset and rotate plane.
        p->gunfx_model.transform 
            = MatrixMultiply(MatrixTranslate(
                        0,
                        0,
                        weapon_model->prjfx_offset
                        ), p->gunfx_model.transform);
        p->gunfx_model.transform = MatrixMultiply(MatrixRotateX(1.5), p->gunfx_model.transform);

        float st = lerp(p->gunfx_timer, 10.0, 0.0);
        p->gunfx_model.transform = MatrixMultiply(MatrixScale(st, st, st), p->gunfx_model.transform);
        
        shader_setu_color(gst, GUNFX_SHADER, U_GUNFX_COLOR, &weapon_model->stats.color);
        DrawMesh(
                p->gunfx_model.meshes[0],
                p->gunfx_model.materials[0],
                p->gunfx_model.transform
                );


        p->gunfx_timer += gst->dt*7.0;
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
            stats_x+220, &stats_y, "Dash Ability", 200, ADD_YINCREMENT,
            p->dash_timer, p->dash_timer_max,
            (p->dash_timer >= p->dash_timer_max) 
            ? (Color){ 125 + 125*(0.5+0.5*sin(gst->time*3.0)), 90, 140, 255} 
            : (Color){ 230, 80, 130, 180 });



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

void player_set_scope_view(struct state_t* gst, struct player_t* p, int view_enabled) {
    p->in_scope_view = view_enabled;

    if(view_enabled) {
        p->cam.fovy = 40.0;
    }
    else {
        p->cam.fovy = 70.0;
    }
    // TODO: Animation.
}


void player_handle_action(struct state_t* gst, struct player_t* p, int iaction_type, int iaction_for, void* ptr) {
    
    if(!IsKeyDown(INTERACTION_KEY)) {
        return;
    }

    switch(iaction_type) {

        case IACTION_HARVEST:
            switch(iaction_for) {
            
                case IACTION_FOR_FRACTAL_TREE:
                    {
                        if(!p->item_in_hands) {
                            return;
                        }
                        if(!p->item_in_hands->is_lqcontainer_item) {
                            printf("Needs a liquid container to be harvested.\n");
                            return;
                        }

                        struct lqcontainer_t* lqcontainer = &p->item_in_hands->lqcontainer;



                        struct fractal_t* fractal = (struct fractal_t*)ptr;
                        //printf("%s %p\n", __func__, fractal);

                        const float factor = gst->dt * 0.05;

                        int harvested = 1;
                        for(int i = 0; i < fractal->num_berries; i++) {
                            if(fractal->berries[i].level <= 0.001) {
                                continue;
                            }
                            harvested = 0;

                            lqcontainer->level += factor * 20.0;
                            fractal->berries[i].level -= factor;
                            printf("\033[90m%f\033[0m\n", fractal->berries[i].level);
                        }
                        
                        printf("Collecting... %f\n", lqcontainer->level);
                        if(harvested) {
                            printf("Done!\n");
                        }
                    }
                    break;

                case IACTION_FOR_MUSHROOM:
                    {
                    }
                    break;

                default:
                    fprintf(stderr, "\033[33m(WARNING) '%s': Invalid action target (%i).\033[0m\n",
                            __func__, iaction_for);
                    break;
            }
            break;


        // ... More action types can be added later :)


        default:
            fprintf(stderr, "\033[33m(WARNING) '%s': Invalid action type (%i).\033[0m\n",
                    __func__, iaction_type);
            break;
    }
}




