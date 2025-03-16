#include <stdio.h>

#include "state.h"
#include "input.h"
#include "util.h"

#include <raymath.h>
#include <rlgl.h>



#include "particle_systems/weapon_psys.h"
#include "particle_systems/prj_envhit_psys.h"
#include "particle_systems/enemy_hit_psys.h"
#include "particle_systems/fog_effect_psys.h"
#include "particle_systems/player_hit_psys.h"
#include "particle_systems/enemy_explosion_psys.h"
#include "particle_systems/water_splash_psys.h"


static void load_texture(struct state_t* gst, const char* filepath, int texid) {
    if(texid >= MAX_TEXTURES) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Texture id is invalid. (%i) for '%s'\033[0m\n",
                __func__, texid, filepath);
        return;
    }
    gst->textures[texid] = LoadTexture(filepath);
    gst->num_textures++;
}

void state_update_shader_uniforms(struct state_t* gst) {

    // --------------------------
    // TODO: OPTIMIZE THIS LATER
    // --------------------------

    // Update Player view position.
    //
    {
        float camposf3[3] = { 
            gst->player.cam.position.x,
            gst->player.cam.position.y,
            gst->player.cam.position.z
        };
        
        SetShaderValue(gst->shaders[DEFAULT_SHADER], 
                gst->shaders[DEFAULT_SHADER].locs[SHADER_LOC_VECTOR_VIEW], camposf3, SHADER_UNIFORM_VEC3);

        SetShaderValue(gst->shaders[FOLIAGE_SHADER], 
                gst->shaders[FOLIAGE_SHADER].locs[SHADER_LOC_VECTOR_VIEW], camposf3, SHADER_UNIFORM_VEC3);

        SetShaderValue(gst->shaders[FOG_PARTICLE_SHADER], 
                gst->shaders[FOG_PARTICLE_SHADER].locs[SHADER_LOC_VECTOR_VIEW], camposf3, SHADER_UNIFORM_VEC3);
        
        SetShaderValue(gst->shaders[WATER_SHADER], 
                gst->shaders[WATER_SHADER].locs[SHADER_LOC_VECTOR_VIEW], camposf3, SHADER_UNIFORM_VEC3);
    }

    // Update camera target and position for post processing.
    {
        const Vector3 camdir = Vector3Normalize(Vector3Subtract(gst->player.cam.target, gst->player.cam.position));
        float camtarget3f[3] = {
            camdir.x, camdir.y, camdir.z
        };
        
        SetShaderValue(gst->shaders[POSTPROCESS_SHADER], 
                gst->fs_unilocs[POSTPROCESS_CAMTARGET_FS_UNILOC], camtarget3f, SHADER_UNIFORM_VEC3);
        
        SetShaderValue(gst->shaders[POSTPROCESS_SHADER], 
                gst->fs_unilocs[POSTPROCESS_CAMPOS_FS_UNILOC], &gst->player.cam.position, SHADER_UNIFORM_VEC3);
    }

    // Update screen size.
    {

        const float screen_size[2] = {
            GetScreenWidth(), GetScreenHeight()
        };

        SetShaderValue(
                gst->shaders[POSTPROCESS_SHADER],
                gst->fs_unilocs[POSTPROCESS_SCREENSIZE_FS_UNILOC],
                screen_size,
                SHADER_UNIFORM_VEC2
                );
    }

    // Update time
    {
        SetShaderValue(gst->shaders[FOLIAGE_SHADER], 
                gst->fs_unilocs[FOLIAGE_SHADER_TIME_FS_UNILOC], &gst->time, SHADER_UNIFORM_FLOAT);
        
        SetShaderValue(gst->shaders[POSTPROCESS_SHADER], 
                gst->fs_unilocs[POSTPROCESS_TIME_FS_UNILOC], &gst->time, SHADER_UNIFORM_FLOAT);
        
        SetShaderValue(gst->shaders[WATER_SHADER], 
                gst->fs_unilocs[WATER_SHADER_TIME_FS_UNILOC], &gst->time, SHADER_UNIFORM_FLOAT);
    }


    {

        float color4f[4] = {
            (float)gst->player.weapon.color.r / 255.0,
            (float)gst->player.weapon.color.g / 255.0,
            (float)gst->player.weapon.color.b / 255.0,
            (float)gst->player.weapon.color.a / 255.0
        };

        SetShaderValue(gst->shaders[GUNFX_SHADER], 
                gst->fs_unilocs[GUNFX_SHADER_COLOR_FS_UNILOC], color4f, SHADER_UNIFORM_VEC4);

    
    }
}


// NOTE: DO NOT RENDER FROM HERE:
void state_update_frame(struct state_t* gst) {
    
    handle_userinput(gst);
    player_update(gst, &gst->player);


    // Enemies.
    for(size_t i = 0; i < gst->num_enemies; i++) {
        update_enemy(gst, &gst->enemies[i]);
    }


    update_psystem(gst, &gst->player.weapon_psys);
    update_psystem(gst, &gst->psystems[ENEMY_LVL0_WEAPON_PSYS]);
    update_psystem(gst, &gst->psystems[PLAYER_PRJ_ENVHIT_PSYS]);
    update_psystem(gst, &gst->psystems[ENEMY_PRJ_ENVHIT_PSYS]);
    update_psystem(gst, &gst->psystems[ENEMY_HIT_PSYS]);
    update_psystem(gst, &gst->psystems[FOG_EFFECT_PSYS]);
    update_psystem(gst, &gst->psystems[PLAYER_HIT_PSYS]);
    update_psystem(gst, &gst->psystems[ENEMY_EXPLOSION_PSYS]);
    update_psystem(gst, &gst->psystems[WATER_SPLASH_PSYS]);
}

#include <stdio.h>
#include <stdlib.h>

static int compare(const void* p1, const void* p2) {
    const struct crithit_marker_t* a = (const struct crithit_marker_t*)p1;
    const struct crithit_marker_t* b = (const struct crithit_marker_t*)p2;
    
    // if a->dst >= b->dst. a goes before b.
    return (a->dst >= b->dst) ? -1 : 1;
}

static void _state_render_crithit_markers(struct state_t* gst) {
    struct crithit_marker_t* marker = NULL;

    struct crithit_marker_t sorted[MAX_RENDER_CRITHITS] = { 0 };
    int num_visible = 0;

    for(size_t i = 0; i < MAX_RENDER_CRITHITS; i++) {
        marker = &gst->crithit_markers[i];
        if(!marker->visible) {
            continue;
        }

        marker->dst = Vector3Distance(gst->player.position, marker->position);

        marker->lifetime += gst->dt;
        if(marker->lifetime >= gst->crithit_marker_maxlifetime) {
            marker->visible = 0;
            continue;
        }
        sorted[num_visible] = *marker;
        num_visible++;
    }

    if(num_visible == 0) {
        return;
    }

    // Sort by distance to fix alpha blending.
    qsort(sorted, num_visible, sizeof *sorted, compare);

    for(size_t i = 0; i < num_visible; i++) {
        struct crithit_marker_t* m = &sorted[i];

        // Interpolate scale and color alpha.

        float t = normalize(m->lifetime, 0, gst->crithit_marker_maxlifetime);
        float scale = lerp(t, 4.0, 0.0);
        float alpha = lerp(t, 255, 0.0);
   

        DrawBillboard(gst->player.cam, 
                gst->textures[CRITICALHIT_TEXID], m->position, scale, 
                (Color){ 180+ sin(gst->time*30)*50, 50, 25, alpha });
    }
}

void state_render_environment(struct state_t* gst) {

    // Render 3D stuff into texture and post process it later.
    BeginTextureMode(gst->env_render_target);
    ClearBackground((Color){
            (0.15) * 255, 
            (0.25) * 255,
            (0.3) * 255,
            255
            });

    BeginMode3D(gst->player.cam);
    {
        // Render debug info if needed. --------
        if(gst->debug) {
            for(size_t i = 0; i < gst->num_enemies; i++) {
                struct enemy_t* ent = &gst->enemies[i];
                if(!ent->alive) {
                    continue;
                }
                
                // Hitboxes
                {
                    for(size_t i = 0; i < ent->num_hitboxes; i++) {
                        DrawCubeWiresV(
                                Vector3Add(ent->position, ent->hitboxes[i].offset),
                                ent->hitboxes[i].size,
                                RED
                                );
                    }
                }

                // Target range.
                {
                    DrawCircle3D(ent->position, ent->target_range, 
                            (Vector3){1.0, 0.0, 0.0}, 90.0, (Color){ 30, 150, 255, 200 });
                    
                    DrawCircle3D(ent->position, ent->target_range, 
                            (Vector3){0.0, 0.0, 0.0}, 0.0, (Color){ 30, 150, 255, 200 });
                    
                    DrawCircle3D(ent->position, ent->target_range, 
                            (Vector3){0.0, 1.0, 0.0}, 90.0, (Color){ 30, 150, 255, 200 });
                }

            }

            //DrawBoundingBox(get_player_boundingbox(&gst->player), GREEN);
        }
        // ------------


        BeginShaderMode(gst->shaders[DEFAULT_SHADER]);

        
        // Enemies.
        for(size_t i = 0; i < gst->num_enemies; i++) {
            struct enemy_t* ent = &gst->enemies[i];
            if(!ent->alive) {
                continue;
            }

            render_enemy(gst, ent);
        }


        render_terrain(gst, &gst->terrain, DEFAULT_SHADER);
        
        render_psystem(gst, &gst->player.weapon_psys, gst->player.weapon.color);
        render_psystem(gst, &gst->psystems[PLAYER_PRJ_ENVHIT_PSYS], gst->player.weapon.color);
        render_psystem(gst, &gst->psystems[ENEMY_LVL0_WEAPON_PSYS], ENEMY_WEAPON_COLOR);
        render_psystem(gst, &gst->psystems[ENEMY_HIT_PSYS], (Color){ 255, 120, 20, 255});
        render_psystem(gst, &gst->psystems[FOG_EFFECT_PSYS], (Color){ 255, 160, 20, 255});
        render_psystem(gst, &gst->psystems[PLAYER_HIT_PSYS], (Color){ 255, 20, 20, 255});
        render_psystem(gst, &gst->psystems[ENEMY_EXPLOSION_PSYS], (Color){ 255, 80, 20, 255});
        render_psystem(gst, &gst->psystems[WATER_SPLASH_PSYS], (Color){ 30, 100, 170, 200});
        
        player_render(gst, &gst->player);
      
        rlDisableDepthMask();
        render_psystem(gst, &gst->psystems[ENEMY_PRJ_ENVHIT_PSYS], ENEMY_WEAPON_COLOR);
        rlEnableDepthMask();
    

        EndShaderMode();
        
        _state_render_crithit_markers(gst);

    }
    EndMode3D();
    EndTextureMode();
    EndShaderMode();


    /*
    // Get environment depth to texture
    BeginTextureMode(gst->depth_texture);
    ClearBackground((Color){255, 255, 255, 255});
    BeginMode3D(gst->player.cam);
    {
        BeginShaderMode(gst->shaders[WDEPTH_SHADER]);


        player_render(gst, &gst->player);
        DrawCube((Vector3){ 20.0, 3.0, -10.0 }, 3.0, 3.0, 3.0, (Color){ 30, 30, 30, 255});

        // Terrain.
        render_terrain(gst, &gst->terrain, WDEPTH_SHADER);

        EndShaderMode();
    }
    EndMode3D();
    EndTextureMode();
    EndShaderMode();
    */

    // Get bloom treshold texture.

    BeginTextureMode(gst->bloomtreshold_target);
    ClearBackground((Color){ 0,0,0, 255 });
    BeginShaderMode(gst->shaders[BLOOM_TRESHOLD_SHADER]);
    {
        DrawTextureRec(
                    gst->env_render_target.texture,
                    (Rectangle) { 
                        0.0, 0.0, 
                        (float)gst->env_render_target.texture.width,
                        -(float)gst->env_render_target.texture.height
                    },
                    (Vector2){ 0.0, 0.0 },
                    WHITE
                    );
    }
    EndShaderMode();
    EndTextureMode();

   
}

void state_setup_all_shaders(struct state_t* gst) {

    SetTraceLogLevel(LOG_ALL);

    // --- Setup Default Shader ---
    {
        Shader* shader = &gst->shaders[DEFAULT_SHADER];
        load_shader(
                "res/shaders/default.vs",
                "res/shaders/default.fs", shader);
        
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(*shader, "matModel");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
    }


    // --- Setup Post Processing Shader ---
    {
        Shader* shader = &gst->shaders[POSTPROCESS_SHADER];
        *shader = LoadShader(
            0, /* use raylibs default vertex shader */ 
            "res/shaders/postprocess.fs"
        );

        gst->fs_unilocs[POSTPROCESS_TIME_FS_UNILOC] = GetShaderLocation(*shader, "time");
        gst->fs_unilocs[POSTPROCESS_SCREENSIZE_FS_UNILOC] = GetShaderLocation(*shader, "screen_size");
        gst->fs_unilocs[POSTPROCESS_PLAYER_HEALTH_FS_UNILOC] = GetShaderLocation(*shader, "health");
        gst->fs_unilocs[POSTPROCESS_CAMTARGET_FS_UNILOC] = GetShaderLocation(*shader, "cam_target");
        gst->fs_unilocs[POSTPROCESS_CAMPOS_FS_UNILOC] = GetShaderLocation(*shader, "cam_pos");
    }


    // --- Setup PRJ_ENVHIT_PSYS_SHADER ---
    {
        Shader* shader = &gst->shaders[PRJ_ENVHIT_PSYS_SHADER];
        load_shader(
                "res/shaders/instance_core.vs",
                "res/shaders/prj_envhit_psys.fs", shader);
       
        shader->locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(*shader, "mvp");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
    }
 

    // --- Setup PLAYER_WEAPON_PSYS_SHADER ---
    {
        Shader* shader = &gst->shaders[BASIC_WEAPON_PSYS_SHADER];
        load_shader(
                "res/shaders/instance_core.vs",
                "res/shaders/basic_weapon_psys.fs", shader);
       
        shader->locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(*shader, "mvp");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
    }
 


    // --- Setup FOLIAGE_SHADER ---
    {
        Shader* shader = &gst->shaders[FOLIAGE_SHADER];
        load_shader(
                "res/shaders/instance_core.vs",
                "res/shaders/foliage.fs", shader);
       
        shader->locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(*shader, "mvp");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
  
        gst->fs_unilocs[FOLIAGE_SHADER_TIME_FS_UNILOC] = GetShaderLocation(*shader, "time");
    }

    // --- Setup FOG_EFFECT_PARTICLE_SHADER ---
    {
        Shader* shader = &gst->shaders[FOG_PARTICLE_SHADER];
        load_shader(
                "res/shaders/instance_core.vs",
                "res/shaders/fog_particle.fs", shader);
       
        shader->locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(*shader, "mvp");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
    }


    // --- Setup Water Shader ---
    {
        Shader* shader = &gst->shaders[WATER_SHADER];
        load_shader(
                "res/shaders/default.vs",
                "res/shaders/water.fs", shader);
        
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(*shader, "matModel");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
       
        gst->fs_unilocs[WATER_SHADER_TIME_FS_UNILOC] = GetShaderLocation(*shader, "time");
    }


    // --- Setup Bloom Treshold Shader ---
    {
        Shader* shader = &gst->shaders[BLOOM_TRESHOLD_SHADER];
        *shader = LoadShader(
            0, /* use raylibs default vertex shader */
            "res/shaders/bloom_treshold.fs"
        );
    }


    // --- Setup Depth value Shader ---
    {
        Shader* shader = &gst->shaders[WDEPTH_SHADER];
        *shader = LoadShader(
            0, /* use raylibs default vertex shader */ 
            "res/shaders/write_depth.fs"
        );
    }


    // --- Setup Gun FX Shader ---
    {
        Shader* shader = &gst->shaders[GUNFX_SHADER];
        *shader = LoadShader(
            0, /* use raylibs default vertex shader */ 
            "res/shaders/gun_fx.fs"
        );
        
        gst->fs_unilocs[GUNFX_SHADER_COLOR_FS_UNILOC] = GetShaderLocation(*shader, "gun_color");
    }



    SetTraceLogLevel(LOG_NONE);
}

void state_setup_all_weapons(struct state_t* gst) {

   
    // Player's weapon.
    gst->player.weapon = (struct weapon_t) {
        .id = PLAYER_WEAPON_ID,
        .accuracy = 9.85,
        .damage = 10.0,
        .critical_chance = 10,
        .critical_mult = 1.85,
        .prj_speed = 450.0,
        .prj_max_lifetime = 5.0,
        .prj_hitbox_size = (Vector3) { 1.0, 1.0, 1.0 },
        .color = (Color) { 20, 255, 200, 255 },
        
        // TODO -----
        .overheat_temp = 100.0,
        .heat_increase = 2.0,
        .cooling_level = 20.0
    };
 

    // Enemy lvl0 weapon.
    gst->enemy_weapons[ENEMY_LVL0_WEAPON] = (struct weapon_t) {
        .id = ENEMY_WEAPON_ID,
        .accuracy = 9.25,
        .damage = 1.0,
        .critical_chance = 7,
        .critical_mult = 5.0,
        .prj_speed = 485.0,
        .prj_max_lifetime = 5.0,
        .prj_hitbox_size = (Vector3) { 1.5, 1.5, 1.5 },
        .color = ENEMY_WEAPON_COLOR,
        .overheat_temp = -1,
    };

}

void state_setup_all_psystems(struct state_t* gst) {

    // Create PLAYER_WEAPON_PSYS
    { // (projectile particle system for player)
        struct psystem_t* psystem = &gst->player.weapon_psys;
        create_psystem(
                gst,
                PSYS_GROUPID_PLAYER,
                PSYS_ONESHOT,
                psystem,
                64,
                weapon_psys_prj_update,
                weapon_psys_prj_init,
                BASIC_WEAPON_PSYS_SHADER
                );

        psystem->particle_mesh = GenMeshSphere(1.25, 16, 16);
        psystem->userptr = &gst->player.weapon;
    }

    // Create PLAYER_PRJ_ENVHIT_PSYS.
    { // (when player's projectile hits environment)
        struct psystem_t* psystem = &gst->psystems[PLAYER_PRJ_ENVHIT_PSYS];
        create_psystem(
                gst,
                PSYS_GROUPID_PLAYER,
                PSYS_ONESHOT,
                psystem,
                64,
                prj_envhit_psys_update,
                prj_envhit_psys_init,
                PRJ_ENVHIT_PSYS_SHADER
                );

        psystem->particle_mesh = GenMeshSphere(0.5, 32, 32);
        psystem->userptr = &gst->player.weapon;
    }

    // Create ENEMY_LVL0_WEAPON_PSYS.
    { // (projectile particle system for enemy lvl 0)
        struct psystem_t* psystem = &gst->psystems[ENEMY_LVL0_WEAPON_PSYS];
        create_psystem(
                gst,
                PSYS_GROUPID_ENEMY,
                PSYS_ONESHOT,
                psystem,
                512,
                weapon_psys_prj_update,
                weapon_psys_prj_init,
                BASIC_WEAPON_PSYS_SHADER
                );

        psystem->particle_mesh = GenMeshSphere(1.5, 16, 16);
        psystem->userptr = &gst->enemy_weapons[ENEMY_LVL0_WEAPON];
    }

    // Create ENEMY_PRJ_ENVHIT_PSYS.
    { // (when enemies projectile hits environment)
        struct psystem_t* psystem = &gst->psystems[ENEMY_PRJ_ENVHIT_PSYS];
        create_psystem(
                gst,
                PSYS_GROUPID_ENEMY,
                PSYS_ONESHOT,
                psystem,
                64,
                prj_envhit_psys_update,
                prj_envhit_psys_init,
                PRJ_ENVHIT_PSYS_SHADER
                );

        psystem->particle_mesh = GenMeshSphere(0.5, 32, 32);
        psystem->userptr = &gst->player.weapon;
    }


    // Create ENEMY_HIT_PSYS.
    { // (when enemy gets hit)
        struct psystem_t* psystem = &gst->psystems[ENEMY_HIT_PSYS];
        create_psystem(
                gst,
                PSYS_GROUPID_ENV,
                PSYS_ONESHOT,
                psystem,
                512,
                enemy_hit_psys_update,
                enemy_hit_psys_init,
                PRJ_ENVHIT_PSYS_SHADER
                );

        psystem->particle_mesh = GenMeshSphere(0.8, 8, 8);
        psystem->userptr = &gst->player.weapon;
    }

    // Create FOG_EFFECT_PSYS.
    { // (environment effect)

        const int num_fog_effect_parts = 700;
        struct psystem_t* psystem = &gst->psystems[FOG_EFFECT_PSYS];
        create_psystem(
                gst,
                PSYS_GROUPID_ENV,
                PSYS_CONTINUOUS,
                psystem,
                num_fog_effect_parts,
                fog_effect_psys_update,
                fog_effect_psys_init,
                FOG_PARTICLE_SHADER
                );

        psystem->particle_mesh = GenMeshSphere(0.5, 4, 4);
        add_particles(gst, psystem, num_fog_effect_parts, (Vector3){0}, (Vector3){0}, NULL, NO_EXTRADATA);
    }

    // Create PLAYER_HIT_PSYS.
    { // (when player gets hit)
        struct psystem_t* psystem = &gst->psystems[PLAYER_HIT_PSYS];
        create_psystem(
                gst,
                PSYS_GROUPID_ENV,
                PSYS_ONESHOT,
                psystem,
                256,
                player_hit_psys_update,
                player_hit_psys_init,
                PRJ_ENVHIT_PSYS_SHADER
                );

        psystem->particle_mesh = GenMeshSphere(0.45, 8, 8);
        psystem->userptr = &gst->player.weapon;
    }


    // Create WATER_SPLASH_PSYS.
    { // (when something hits water)
        struct psystem_t* psystem = &gst->psystems[WATER_SPLASH_PSYS];
        create_psystem(
                gst,
                PSYS_GROUPID_ENV,
                PSYS_ONESHOT,
                psystem,
                256,
                water_splash_psys_update,
                water_splash_psys_init,
                PRJ_ENVHIT_PSYS_SHADER
                );

        psystem->particle_mesh = GenMeshSphere(0.45, 8, 8);
        psystem->userptr = &gst->player.weapon;
    }

    // Create ENEMY_EXPLOSION_PSYS.
    { // (when enemy dies it "explodes")
        struct psystem_t* psystem = &gst->psystems[ENEMY_EXPLOSION_PSYS];
        create_psystem(
                gst,
                PSYS_GROUPID_ENV,
                PSYS_ONESHOT,
                psystem,
                5000,
                enemy_explosion_psys_update,
                enemy_explosion_psys_init,
                PRJ_ENVHIT_PSYS_SHADER
                );

        psystem->particle_mesh = GenMeshSphere(0.6, 8, 8);
    }
}

void state_setup_all_textures(struct state_t* gst) {
    load_texture(gst, "res/textures/grid_4x4.png", GRID4x4_TEXID);
    load_texture(gst, "res/textures/grid_6x6.png", GRID6x6_TEXID);
    load_texture(gst, "res/textures/grid_9x9.png", GRID9x9_TEXID);
    load_texture(gst, "res/textures/gun_0.png", GUN_0_TEXID);
    load_texture(gst, "res/textures/enemy_lvl0.png", ENEMY_LVL0_TEXID);
    load_texture(gst, "res/textures/arms.png", PLAYER_ARMS_TEXID);
    load_texture(gst, "res/textures/critical_hit.png", CRITICALHIT_TEXID);
    load_texture(gst, "res/textures/tree_bark.png", TREEBARK_TEXID);
    load_texture(gst, "res/textures/leaf.jpg", LEAF_TEXID);
    load_texture(gst, "res/textures/rock_type0.jpg", ROCK_TEXID);
    load_texture(gst, "res/textures/moss2.png", MOSS_TEXID);
    load_texture(gst, "res/textures/grass.png", GRASS_TEXID);
    load_texture(gst, "res/textures/gun_fx.png", GUNFX_TEXID);

   
    SetTextureWrap(gst->textures[LEAF_TEXID], TEXTURE_WRAP_MIRROR_REPEAT);
    SetTextureWrap(gst->textures[ROCK_TEXID], TEXTURE_WRAP_MIRROR_REPEAT);
    SetTextureWrap(gst->textures[MOSS_TEXID], TEXTURE_WRAP_MIRROR_REPEAT);
    SetTextureWrap(gst->textures[GRASS_TEXID], TEXTURE_WRAP_CLAMP);
}


void state_setup_all_sounds(struct state_t* gst) {
    gst->has_audio = 0;
    InitAudioDevice();

    if(!IsAudioDeviceReady()) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Failed to initialize audio device\033[0m\n",
                __func__);
        return;
    }

    gst->sounds[PLAYER_GUN_SOUND] = LoadSound("res/audio/player_gun.wav");
    gst->sounds[ENEMY_HIT_SOUND_0] = LoadSound("res/audio/enemy_hit_0.wav");
    gst->sounds[ENEMY_HIT_SOUND_1] = LoadSound("res/audio/enemy_hit_1.wav");
    gst->sounds[ENEMY_HIT_SOUND_2] = LoadSound("res/audio/enemy_hit_2.wav");
    gst->sounds[ENEMY_GUN_SOUND] = LoadSound("res/audio/enemy_gun.wav");
    gst->sounds[PRJ_ENVHIT_SOUND] = LoadSound("res/audio/envhit.wav");
    gst->sounds[PLAYER_HIT_SOUND] = LoadSound("res/audio/playerhit.wav");
    gst->sounds[ENEMY_EXPLOSION_SOUND] = LoadSound("res/audio/enemy_explosion.wav");
    

    SetMasterVolume(30.0);
    gst->has_audio = 1;
}




void state_setup_all_enemy_models(struct state_t* gst) {

    load_enemy_model(gst, ENEMY_LVL0, "res/models/enemy_lvl0.glb", ENEMY_LVL0_TEXID);


}


void state_delete_all_shaders(struct state_t* gst) {
    for(int i = 0; i < MAX_SHADERS; i++) {
        UnloadShader(gst->shaders[i]);
    }
    
    printf("\033[35m -> Deleted all Shaders\033[0m\n");
}

void state_delete_all_psystems(struct state_t* gst) {
    delete_psystem(&gst->player.weapon_psys);
    for(int i = 0; i < MAX_PSYSTEMS; i++) {
        delete_psystem(&gst->psystems[i]);
    }
   
    printf("\033[35m -> Deleted all Particle systems\033[0m\n");
}


void state_delete_all_textures(struct state_t* gst) {
    // Delete all textures.
    for(unsigned int i = 0; i < gst->num_textures; i++) {
        UnloadTexture(gst->textures[i]);
    }

    printf("\033[35m -> Deleted all Textures\033[0m\n");
}

void state_delete_all_sounds(struct state_t* gst) {
    if(gst->has_audio) {
        for(int i = 0; i < MAX_SOUNDS; i++) {
            UnloadSound(gst->sounds[i]);
        }    
    }
    if(IsAudioDeviceReady()) {
        CloseAudioDevice();
    }
    
    printf("\033[35m -> Deleted all Sounds\033[0m\n");
}

void state_delete_all_enemy_models(struct state_t* gst) {

    for(int i = 0; i < MAX_ENEMY_MODELS; i++) {
        if(IsModelValid(gst->enemy_models[i])) {
            UnloadModel(gst->enemy_models[i]);
        }
    }


    printf("\033[35m -> Deleted all Enemy models\033[0m\n");
}

void state_add_crithit_marker(struct state_t* gst, Vector3 position) {
    struct crithit_marker_t* marker = &gst->crithit_markers[gst->num_crithit_markers];

    marker->visible = 1;
    marker->lifetime = 0.0;
    marker->position = position;

    const float r = 10.0;
    marker->position.x += RSEEDRANDOMF(-r, r);
    marker->position.z += RSEEDRANDOMF(-r, r);
    marker->position.y += RSEEDRANDOMF(r*1.5, r*2)*0.2;

    gst->num_crithit_markers++;
    if(gst->num_crithit_markers >= MAX_RENDER_CRITHITS) {
        gst->num_crithit_markers = 0;
    }
}



