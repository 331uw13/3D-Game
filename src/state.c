#include <stdio.h>
#include <stdlib.h>

#include "state.h"
#include "input.h"
#include "util.h"

#include <raymath.h>
#include <rlgl.h>


#include "particle_systems/weapon_psys.h"
#include "particle_systems/projectile_envhit_psys.h"
#include "particle_systems/enemy_hit_psys.h"
#include "particle_systems/fog_effect_psys.h"
#include "particle_systems/player_hit_psys.h"
#include "particle_systems/explosion_psys.h"
#include "particle_systems/water_splash_psys.h"
#include "particle_systems/enemy_gunfx_psys.h"
#include "particle_systems/cloud_psys.h"
#include "particle_systems/prj_trail_psys.h"


void state_setup_gbuffer(struct state_t* gst) {
    gst->gbuffer = (struct gbuffer_t) { 0 };

    gst->gbuffer.framebuffer = rlLoadFramebuffer();
   
    printf("Creating gbuffer %ix%i\n", gst->scrn_w, gst->scrn_h);
    if(!gst->gbuffer.framebuffer) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Failed to create framebuffer\033[0m\n",
                __func__);
        return;
    }

    rlEnableFramebuffer(gst->gbuffer.framebuffer);

    // Use 32 bits per channel to avoid floating point precision loss.
    
    // Positions.
    gst->gbuffer.position_tex = rlLoadTexture(NULL, gst->scrn_w, gst->scrn_h, 
                RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32, 1);
    
    // Normals.
    gst->gbuffer.normal_tex = rlLoadTexture(NULL, gst->scrn_w, gst->scrn_h, 
                RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32, 1);
    
    // Diffuse specular colors.
    gst->gbuffer.difspec_tex = rlLoadTexture(NULL, gst->scrn_w, gst->scrn_h, 
                RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8, 1);

    // Depth
    gst->gbuffer.depth_tex = rlLoadTexture(NULL, gst->scrn_w, gst->scrn_h, 
                RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32, 1);




    gst->gbuffer.depthbuffer = rlLoadTextureDepth(gst->scrn_w, gst->scrn_h, 1);


    rlActiveDrawBuffers(4);

    // Attach textures to the framebuffer.
    rlFramebufferAttach(gst->gbuffer.framebuffer, gst->gbuffer.position_tex,
            RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
    
    rlFramebufferAttach(gst->gbuffer.framebuffer, gst->gbuffer.normal_tex,
            RL_ATTACHMENT_COLOR_CHANNEL1, RL_ATTACHMENT_TEXTURE2D, 0);

    rlFramebufferAttach(gst->gbuffer.framebuffer, gst->gbuffer.difspec_tex,
            RL_ATTACHMENT_COLOR_CHANNEL2, RL_ATTACHMENT_TEXTURE2D, 0);

    rlFramebufferAttach(gst->gbuffer.framebuffer, gst->gbuffer.depth_tex,
            RL_ATTACHMENT_COLOR_CHANNEL3, RL_ATTACHMENT_TEXTURE2D, 0);

    // Attach depth buffer.
    rlFramebufferAttach(gst->gbuffer.framebuffer, gst->gbuffer.depthbuffer,
            RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);


    if(!rlFramebufferComplete(gst->gbuffer.framebuffer)) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Framebuffer is not complete!\033[0m\n",
                __func__);
    }

}

void state_setup_ssao(struct state_t* gst) {

    Shader shader = gst->shaders[POSTPROCESS_SHADER];
    
    for(size_t i = 0; i < SSAO_KERNEL_SIZE; i++) {

        // Get scale factor to move points closer to the center.
        float scale = (float)i / (float)SSAO_KERNEL_SIZE;
        scale = lerp(scale*scale, 0.1, 1.0);

        Vector3 sample = (Vector3) {
            RSEEDRANDOMF(-1.0, 1.0) * scale,
            RSEEDRANDOMF(-1.0, 1.0) * scale,
            RSEEDRANDOMF( 0.0, 1.0) * scale // 0.0 - 1.0 for z to because we want hemisphere sample kernel.
        };
  

        gst->ssao_kernel[i] = sample;
    }

    int width = 32;
    int height = 32;
    Color *pixels = (Color *)malloc(width*height*sizeof(Color));

    for (int i = 0; i < width*height; i++) {
        pixels[i] = (Color) {
            GetRandomValue(0, 255),
            GetRandomValue(0, 255),
            0, // Rotating around z axis.
            0
        };
    }

    Image image = (Image) {
        .data = pixels,
        .width = width,
        .height = height,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };

    gst->ssao_noise_tex = LoadTextureFromImage(image);
    SetTextureWrap(gst->ssao_noise_tex, TEXTURE_WRAP_MIRROR_REPEAT);

    free(pixels);
    /*
    Image img = GenImageWhiteNoise(16, 16, 0.5);
    gst->ssao_noise_tex = LoadTextureFromImage(img);
    UnloadImage(img);
    */
}

void state_delete_gbuffer(struct state_t* gst) {
    rlUnloadFramebuffer(gst->gbuffer.framebuffer);
    rlUnloadTexture(gst->gbuffer.depthbuffer);
    rlUnloadTexture(gst->gbuffer.position_tex);
    rlUnloadTexture(gst->gbuffer.normal_tex);
    rlUnloadTexture(gst->gbuffer.difspec_tex);
}

void state_create_ubo(struct state_t* gst, int ubo_index, int binding_point, size_t size) {
    gst->ubo[ubo_index] = 0;

    glGenBuffers(1, &gst->ubo[ubo_index]);
    glBindBuffer(GL_UNIFORM_BUFFER, gst->ubo[ubo_index]);
    glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, gst->ubo[ubo_index]);
    glBindBufferRange(GL_UNIFORM_BUFFER, binding_point, gst->ubo[ubo_index], 0, size);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void state_setup_render_targets(struct state_t* gst) {

    gst->scrn_w = GetScreenWidth();
    gst->scrn_h = GetScreenHeight();

    gst->env_render_target = LoadRenderTexture(gst->scrn_w, gst->scrn_h);
    gst->bloomtresh_target = LoadRenderTexture(gst->scrn_w, gst->scrn_h);
    gst->ssao_target = LoadRenderTexture(gst->scrn_w, gst->scrn_h);
    
}

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

        SetShaderValue(
                gst->shaders[POWERUP_SHOP_BG_SHADER],
                GetShaderLocation(gst->shaders[POWERUP_SHOP_BG_SHADER], "screen_size"),
                screen_size,
                SHADER_UNIFORM_VEC2
                );
    }
    

    {

        float blur_effect = 0.62;
        if(gst->player.any_gui_open) {
            blur_effect = 0.1;
        }

        SetShaderValue(gst->shaders[POSTPROCESS_SHADER], 
                GetShaderLocation(gst->shaders[POSTPROCESS_SHADER], "blur_effect"),
                &blur_effect, SHADER_UNIFORM_FLOAT);

    }
    // Update time
    {
        SetShaderValue(gst->shaders[FOLIAGE_SHADER], 
                gst->fs_unilocs[FOLIAGE_SHADER_TIME_FS_UNILOC], &gst->time, SHADER_UNIFORM_FLOAT);
        
        SetShaderValue(gst->shaders[POSTPROCESS_SHADER], 
                gst->fs_unilocs[POSTPROCESS_TIME_FS_UNILOC], &gst->time, SHADER_UNIFORM_FLOAT);
        
        SetShaderValue(gst->shaders[WATER_SHADER], 
                gst->fs_unilocs[WATER_SHADER_TIME_FS_UNILOC], &gst->time, SHADER_UNIFORM_FLOAT);
    

        SetShaderValue(gst->shaders[POWERUP_SHOP_BG_SHADER], 
                GetShaderLocation(gst->shaders[POWERUP_SHOP_BG_SHADER], "time"),
                &gst->time, SHADER_UNIFORM_FLOAT);

    }

    // Water level
    SetShaderValue(gst->shaders[DEFAULT_SHADER], 
            GetShaderLocation(gst->shaders[DEFAULT_SHADER], "water_level"), &gst->terrain.water_ylevel, SHADER_UNIFORM_FLOAT);
    
    SetShaderValue(gst->shaders[DEFAULT_SHADER], 
            GetShaderLocation(gst->shaders[DEFAULT_SHADER], "time"), &gst->time, SHADER_UNIFORM_FLOAT);
    
}


// NOTE: DO NOT RENDER FROM HERE:
void state_update_frame(struct state_t* gst) {
    
    gst->player.any_gui_open = (
            gst->menu_open 
            || gst->devmenu_open
            || gst->player.inventory.open
            || gst->player.powerup_shop.open
         );

    handle_userinput(gst);
    
    if(gst->menu_open) {
        return;
    }

    

    // Enemies.
    if(!gst->player.powerup_shop.open) {
        for(size_t i = 0; i < gst->num_enemies; i++) {
            update_enemy(gst, &gst->enemies[i]);
        }

        update_enemy_spawn_systems(gst); 
    }
    
    update_natural_item_spawns(gst);

    // (updated only if needed)
    update_psystem(gst, &gst->psystems[PLAYER_WEAPON_PSYS]);
    update_psystem(gst, &gst->psystems[ENEMY_WEAPON_PSYS]);
    update_psystem(gst, &gst->psystems[PROJECTILE_ENVHIT_PSYS]);
    update_psystem(gst, &gst->psystems[ENEMY_HIT_PSYS]);
    update_psystem(gst, &gst->psystems[FOG_EFFECT_PSYS]);
    update_psystem(gst, &gst->psystems[PLAYER_HIT_PSYS]);
    update_psystem(gst, &gst->psystems[EXPLOSION_PSYS]);
    update_psystem(gst, &gst->psystems[WATER_SPLASH_PSYS]);
    update_psystem(gst, &gst->psystems[ENEMY_GUNFX_PSYS]);
    update_psystem(gst, &gst->psystems[CLOUD_PSYS]);
    update_psystem(gst, &gst->psystems[PRJ_TRAIL_PSYS]);

    update_items(gst);
    update_decay_lights(gst);
    update_inventory(gst, &gst->player);
    
    player_update(gst, &gst->player);

    // Update xp level.
    if(gst->xp_value_add != 0) {
        gst->xp_update_timer += gst->dt;

        if(gst->xp_update_timer >= 0.001) {
            gst->xp_update_timer = 0.0;

            if(gst->xp_value_add > 0) {
                gst->player.xp++;
                gst->xp_value_add--;
            }
            else {
                gst->player.xp--;
                gst->xp_value_add++;
            }
        }
    }
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
        load_shader(
                "res/shaders/default.vs",
                "res/shaders/postprocess.fs", shader);

        gst->fs_unilocs[POSTPROCESS_TIME_FS_UNILOC] = GetShaderLocation(*shader, "time");
        gst->fs_unilocs[POSTPROCESS_SCREENSIZE_FS_UNILOC] = GetShaderLocation(*shader, "screen_size");
        gst->fs_unilocs[POSTPROCESS_PLAYER_HEALTH_FS_UNILOC] = GetShaderLocation(*shader, "health");
        gst->fs_unilocs[POSTPROCESS_CAMTARGET_FS_UNILOC] = GetShaderLocation(*shader, "cam_target");
        gst->fs_unilocs[POSTPROCESS_CAMPOS_FS_UNILOC] = GetShaderLocation(*shader, "cam_pos");
    }

    // --- Setup POWERUP_SHOP_BG_SHADER ---
    {
        Shader* shader = &gst->shaders[POWERUP_SHOP_BG_SHADER];
        load_shader(
                "res/shaders/default.vs",
                "res/shaders/powerup_shop_bg.fs", shader);
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
 


    // --- Setup EXPLOSION_PSYS_SHADER ---
    {
        Shader* shader = &gst->shaders[EXPLOSION_PSYS_SHADER];
        load_shader(
                "res/shaders/instance_core.vs",
                "res/shaders/explosion_psys.fs", shader);
       
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


    // --- Setup Gun FX Shader ---
    {
        Shader* shader = &gst->shaders[GUNFX_SHADER];
        *shader = LoadShader(
            "res/shaders/default.vs", /* use raylibs default vertex shader */ 
            "res/shaders/player_gunfx.fs"
        );
        
        gst->fs_unilocs[GUNFX_SHADER_COLOR_FS_UNILOC] = GetShaderLocation(*shader, "player_gun_color");
    }

    // --- Setup Enemy Gun FX Shader ---
    {
        Shader* shader = &gst->shaders[ENEMY_GUNFX_SHADER];
        *shader = LoadShader(
            "res/shaders/instance_core.vs",
            "res/shaders/enemy_gunfx.fs"
        );
       
        shader->locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(*shader, "mvp");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
    }


    {
        Shader* shader = &gst->shaders[PLAYER_HIT_SHADER];
        load_shader(
            "res/shaders/instance_core.vs",
            "res/shaders/player_hit.fs",
            shader
        );
       
        shader->locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(*shader, "mvp");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
    }


    // --- Setup gbuffer shader ---
    {
        Shader* shader = &gst->shaders[GBUFFER_SHADER];
        *shader = LoadShader(
            "res/shaders/default.vs", 
            "res/shaders/gbuffer.fs"
        );
    }

    // --- Setup gbuffer for instanced rendering ---
    {
        Shader* shader = &gst->shaders[GBUFFER_INSTANCE_SHADER];
        *shader = LoadShader(
            "res/shaders/instance_core.vs", 
            "res/shaders/gbuffer.fs"
        );

        shader->locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(*shader, "mvp");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
    }


    // --- Setup ssao shader ---
    {
        Shader* shader = &gst->shaders[SSAO_SHADER];
        *shader = LoadShader(
            "res/shaders/default.vs", 
            "res/shaders/ssao.fs"
        );
    }

    


    SetTraceLogLevel(LOG_NONE);
}

void state_setup_all_weapons(struct state_t* gst) {

   
    // Player's weapon.
    gst->player.weapon = (struct weapon_t) {
        .gid = PLAYER_WEAPON_GID,
        .accuracy = 8.35,
        .damage = 10.0,
        .critical_chance = 10,
        .critical_mult = 1.85,
        .prj_speed = 530.0,
        .prj_max_lifetime = 15.0,
        .prj_hitbox_size = (Vector3) { 1.0, 1.0, 1.0 },
        .prj_scale = 1.0,
        .color = (Color) { 20, 255, 200, 255 },
        
        // TODO -----
        .overheat_temp = 100.0,
        .heat_increase = 2.0,
        .cooling_level = 20.0
    };
 

    // Enemy lvl0 weapon.
    gst->enemy_weapons[ENEMY_LVL0_WEAPON] = (struct weapon_t) {
        .gid = ENEMY_WEAPON_GID,
        .accuracy = 9.25,
        .damage = 15.0,
        .critical_chance = 15,
        .critical_mult = 5.0,
        .prj_speed = 530.0,
        .prj_max_lifetime = 5.0,
        .prj_hitbox_size = (Vector3) { 1.5, 1.5, 1.5 },
        .prj_scale = 1.0,
        .color = (Color){ 255, 20, 50, 200 },
        .overheat_temp = -1,
    };

    // Enemy lvl1 weapon.
    gst->enemy_weapons[ENEMY_LVL1_WEAPON] = (struct weapon_t) {
        .gid = ENEMY_WEAPON_GID,
        .accuracy = 7.535,
        .damage = 25.0,
        .critical_chance = 5,
        .critical_mult = 7.0,
        .prj_speed = 685.0,
        .prj_max_lifetime = 5.0,
        .prj_hitbox_size = (Vector3) { 1.5, 1.5, 1.5 },
        .prj_scale = 1.0,
        .color = (Color){ 255, 20, 255, 200 },
        .overheat_temp = -1,
    };
}

void state_setup_all_psystems(struct state_t* gst) {

    { 
        // (projectile particle system for player)
        struct psystem_t* psystem = &gst->psystems[PLAYER_WEAPON_PSYS];
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
        setup_psystem_color_vbo(gst, psystem);
    }

    // Create ENEMY_WEAPON_PSYS.
    { // (projectile particle system for enemies)
        struct psystem_t* psystem = &gst->psystems[ENEMY_WEAPON_PSYS];
        create_psystem(
                gst,
                PSYS_GROUPID_ENEMY,
                PSYS_ONESHOT,
                psystem,
                64,
                weapon_psys_prj_update,
                weapon_psys_prj_init,
                BASIC_WEAPON_PSYS_SHADER
                );

        psystem->particle_mesh = GenMeshSphere(1.5, 16, 16);
        setup_psystem_color_vbo(gst, psystem);
    }


    // Create PROJECTILE_ENVHIT_PSYS.
    { 
        struct psystem_t* psystem = &gst->psystems[PROJECTILE_ENVHIT_PSYS];
        create_psystem(
                gst,
                PSYS_GROUPID_ENV,
                PSYS_ONESHOT,
                psystem,
                64,
                projectile_envhit_psys_update,
                projectile_envhit_psys_init,
                PRJ_ENVHIT_PSYS_SHADER
                );

        psystem->particle_mesh = GenMeshSphere(0.5, 32, 32);
        setup_psystem_color_vbo(gst, psystem);
    }



    // Create PRJ_TRAIL_PSYS
    { 
        struct psystem_t* psystem = &gst->psystems[PRJ_TRAIL_PSYS];
        create_psystem(
                gst,
                PSYS_GROUPID_ENV,
                PSYS_ONESHOT,
                psystem,
                1024,
                prj_trail_psys_update,
                prj_trail_psys_init,
                BASIC_WEAPON_PSYS_SHADER
                );

        psystem->particle_mesh = GenMeshSphere(1.25, 16, 16);
        setup_psystem_color_vbo(gst, psystem);
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
        setup_psystem_color_vbo(gst, psystem);
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
                PLAYER_HIT_SHADER
                );

        psystem->particle_mesh = GenMeshSphere(0.75, 8, 8);
        setup_psystem_color_vbo(gst, psystem);
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
        add_particles(gst, psystem, num_fog_effect_parts, 
                (Vector3){0}, (Vector3){0}, (Color){0},
                NULL, NO_EXTRADATA, NO_IDB);
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

    // Create EXPLOSION_PSYS.
    {
        struct psystem_t* psystem = &gst->psystems[EXPLOSION_PSYS];
        create_psystem(
                gst,
                PSYS_GROUPID_ENV,
                PSYS_ONESHOT,
                psystem,
                2048,
                explosion_psys_update,
                explosion_psys_init,
                EXPLOSION_PSYS_SHADER
                );

        psystem->particle_mesh = GenMeshSphere(0.6, 8, 8);
        setup_psystem_color_vbo(gst, psystem);
    }

    
    // Create ENEMY_GUNFX_PSYS.
    { // (gun fx)
        struct psystem_t* psystem = &gst->psystems[ENEMY_GUNFX_PSYS];
        create_psystem(
                gst,
                PSYS_GROUPID_ENV,
                PSYS_ONESHOT,
                psystem,
                64,
                enemy_gunfx_psys_update,
                enemy_gunfx_psys_init,
                ENEMY_GUNFX_SHADER
                );

        //psystem->particle_mesh = GenMeshSphere(0.6, 8, 8);
        psystem->particle_mesh = GenMeshPlane(1.0, 1.0, 1, 1);
        psystem->particle_material.maps[MATERIAL_MAP_DIFFUSE].texture 
            = gst->textures[GUNFX_TEXID];
    
        setup_psystem_color_vbo(gst, psystem);
    }


    // Create CLOUD_PSYS.
    {
        const size_t num_cloud_parts = 200;
        struct psystem_t* psystem = &gst->psystems[CLOUD_PSYS];
        create_psystem(
                gst,
                PSYS_GROUPID_ENV,
                PSYS_CONTINUOUS,
                psystem,
                num_cloud_parts,
                cloud_psys_update,
                cloud_psys_init,
                FOG_PARTICLE_SHADER
                );

        psystem->particle_mesh = GenMeshCube(30, 10, 80);
        add_particles(gst, psystem, num_cloud_parts, 
                (Vector3){0}, (Vector3){0}, (Color){0},
                NULL, NO_EXTRADATA, NO_IDB);
    }

}

void state_setup_all_textures(struct state_t* gst) {

    SetTraceLogLevel(LOG_ALL);

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
    load_texture(gst, "res/textures/apple_inv.png", APPLE_INV_TEXID);
    load_texture(gst, "res/textures/apple.png", APPLE_TEXID);
    load_texture(gst, "res/textures/blue_metal.png", METALPIECE_TEXID);
    load_texture(gst, "res/textures/metalpiece_inv.png", METALPIECE_INV_TEXID);
    load_texture(gst, "res/textures/cloth.png", PLAYER_HANDS_TEXID);
    load_texture(gst, "res/textures/player_skin.png", PLAYER_SKIN_TEXID);
    load_texture(gst, "res/textures/gun_metal.png", METAL2_TEXID);
    load_texture(gst, "res/textures/enemy_lvl1.png", ENEMY_LVL1_TEXID);
    
    SetTraceLogLevel(LOG_NONE);

   
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
    gst->sounds[POWERUP_SOUND] = LoadSound("res/audio/powerup.wav");
    

    SetMasterVolume(30.0);
    gst->has_audio = 1;
}




void state_setup_all_enemy_models(struct state_t* gst) {

    for(size_t i = 0; i < MAX_ALL_ENEMIES; i++) {
        gst->enemies[i].alive = 0;
    }

    load_enemy_model(gst, ENEMY_LVL0, "res/models/enemy_lvl0.glb", ENEMY_LVL0_TEXID);
    load_enemy_model(gst, ENEMY_LVL1, "res/models/enemy_lvl1.glb", ENEMY_LVL1_TEXID);

    // ...
}

void state_setup_all_item_models(struct state_t* gst) {
    for(size_t i = 0; i < MAX_ALL_ITEMS; i++) {
        gst->items[i].enabled = 0;
    }
    
    load_item_model(gst, ITEM_APPLE, "res/models/apple.glb", APPLE_TEXID);
    load_item_model(gst, ITEM_METALPIECE, "res/models/metal_piece.glb", METALPIECE_TEXID);
    
    // ...

}


void state_delete_all_shaders(struct state_t* gst) {
    for(int i = 0; i < MAX_SHADERS; i++) {
        UnloadShader(gst->shaders[i]);
    }
    
    printf("\033[35m -> Deleted all Shaders\033[0m\n");
}

void state_delete_all_psystems(struct state_t* gst) {
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

void state_delete_all_item_models(struct state_t* gst) {

    for(int i = 0; i < MAX_ITEM_MODELS; i++) {
        if(IsModelValid(gst->item_models[i])) {
            UnloadModel(gst->item_models[i]);
        }
    }


    printf("\033[35m -> Deleted all Item models\033[0m\n");
}


/*
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
*/

void set_fog_settings(struct state_t* gst, struct fog_t* fog) {
    glBindBuffer(GL_UNIFORM_BUFFER, gst->ubo[FOG_UBO]);

    gst->render_bg_color = (Color) {
        fog->color_far.r * 0.6,
        fog->color_far.g * 0.6,
        fog->color_far.b * 0.6,
        255
    };

    float near_color4f[4] = {
        (float)fog->color_near.r / 255.0,
        (float)fog->color_near.g / 255.0,
        (float)fog->color_near.b / 255.0,
        1.0
    };

    float far_color4f[4] = {
        (float)fog->color_far.r / 255.0,
        (float)fog->color_far.g / 255.0,
        (float)fog->color_far.b / 255.0,
        1.0
    };

    float settings[4] = { 0 };


    if(fog->mode == FOG_MODE_TORENDERDIST) {
        float test = 1.0 / (gst->render_dist-1000.0);
        test = pow(test, exp(test));
        settings[0] = test;
    }




    printf("'%s': New fog density = %f\n", __func__, settings[0]);

    size_t offset;
    size_t size = sizeof(float)*4;

    offset = 0;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, settings);

    offset = 16;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, near_color4f);

    offset = 32;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, far_color4f);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);


    /*
    gst->render_bg_color = (Color) {
        gst->fog.color_far.r * 0.6,
        gst->fog.color_far.g * 0.6,
        gst->fog.color_far.b * 0.6,
        255
    };


    int max_far = 100;
    gst->fog.color_far.r = CLAMP(gst->fog.color_far.r, 0, max_far);
    gst->fog.color_far.g = CLAMP(gst->fog.color_far.g, 0, max_far);
    gst->fog.color_far.b = CLAMP(gst->fog.color_far.b, 0, max_far);

    float near_color[4] = {
        (float)gst->fog.color_near.r / 255.0,
        (float)gst->fog.color_near.g / 255.0,
        (float)gst->fog.color_near.b / 255.0,
        1.0
    };

    float far_color[4] = {
        (float)gst->fog.color_far.r / 255.0,
        (float)gst->fog.color_far.g / 255.0,
        (float)gst->fog.color_far.b / 255.0,
        1.0
    };

    size_t offset;
    size_t size;
    size = sizeof(float)*4;

    // DENSITY
   
    float density[4] = {
        // Convert fog density to more friendly scale.
        // Otherwise it is very exponental with very very samll numbers.
        0.0, //map(log(CLAMP(gst->fog_density, FOG_MIN, FOG_MAX)), FOG_MIN, FOG_MAX, 0.0015, 0.02),
        0.0,
        0.0,  // Maybe adding more settings later.
        0.0
    };


    offset = 0;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, density);

    // NEAR COLOR
    offset = 16;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, near_color);

    // FAR COLOR
    offset = 16*2;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, far_color);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
     */
}


static float get_explosion_effect(Vector3 exp_pos, Vector3 p, float radius) {
    float dist = Vector3Distance(exp_pos, p);
    float effect = CLAMP(normalize(dist, radius, 0.0), 0.0, 1.0);

    return effect * effect;
}


void create_explosion(struct state_t* gst, Vector3 position, float damage, float radius) {

    // Add particles for explosion.

    add_particles(gst,
            &gst->psystems[EXPLOSION_PSYS],
            GetRandomValue(20, 30),
            position,(Vector3){0},(Color){0},
            &radius, HAS_EXTRADATA, PART_IDB_SMOKE);

    add_particles(gst,
            &gst->psystems[EXPLOSION_PSYS],
            GetRandomValue(150, 200),
            position,(Vector3){0},(Color){0},
            &radius, HAS_EXTRADATA, PART_IDB_EXPLOSION);


    // Add light from explosion.

    struct light_t exp_light = (struct light_t) {
        .type = LIGHT_POINT,
        .enabled = 1,
        .position = (Vector3){ position.x, position.y+10, position.z },
        .color = (Color){ 200, 50, 20, 255 },
        .strength = 50.0,
        .radius = 10.0,
        .index = gst->next_explosion_light_index
    };

    set_light(gst, &exp_light, LIGHTS_UBO);
    add_decay_light(gst, &exp_light, 8.0);

    gst->next_explosion_light_index++;
    if(gst->next_explosion_light_index >= MAX_EXPLOSION_LIGHTS) {
        gst->next_explosion_light_index = MAX_STATIC_LIGHTS;
    }
    


    SetSoundVolume(gst->sounds[ENEMY_EXPLOSION_SOUND], get_volume_dist(gst->player.position, position));
    SetSoundPitch(gst->sounds[ENEMY_EXPLOSION_SOUND], 1.0 - RSEEDRANDOMF(0.0, 0.3));
    PlaySound(gst->sounds[ENEMY_EXPLOSION_SOUND]);


    // Calculate explosion effects to player.
    {
        float effect = get_explosion_effect(position, gst->player.position, radius);

        if(effect > 0.0) {
            float damage_to_player = effect * damage;
            printf("Explosion damage to player: %f\n", damage_to_player);
            player_damage(gst, &gst->player, damage_to_player);

            // Apply external force from explosion to player.
            Vector3 force_dir = Vector3Normalize(Vector3Subtract(gst->player.position, position));
            force_dir = Vector3Scale(force_dir, damage_to_player*0.5);
            player_apply_force(gst, &gst->player, force_dir);
        }
    }


    // Calculate explosion effects to enemies nearby.

    for(size_t i = 0; i < gst->num_enemies; i++) {
        struct enemy_t* ent = &gst->enemies[i];
        if(!ent->alive) {
            continue;
        }

        float effect_to_ent = get_explosion_effect(position, ent->position, radius);
        float exp_damage_to_ent = effect_to_ent * damage;
        float exp_knockback_to_ent = effect_to_ent * 10.0;

        if(effect_to_ent <= 0.0) {
            continue;
        }
        printf("Explosion Damage to ent: %0.2f\n", exp_damage_to_ent);
        struct hitbox_t* hitbox = &ent->hitboxes[HITBOX_BODY];

        enemy_damage(
                gst,
                ent,
                exp_damage_to_ent,
                hitbox,
                Vector3Add(ent->position, hitbox->offset),
                Vector3Subtract(ent->position, position),
                exp_knockback_to_ent);
    }
}


void set_render_dist(struct state_t* gst, float new_dist) {
    new_dist = CLAMP(new_dist, MIN_RENDERDIST, MAX_RENDERDIST);

    gst->render_dist = new_dist;
    rlSetClipPlanes(0.160000, gst->render_dist+3000.0);

    gst->menu_slider_render_dist_v = gst->render_dist;

    // Figure out how many chunks may be rendered at once.
    // This is bad because it is not accurate at all... but will do for now.
    // Also doesnt take in count the "frustrum culling" for chunks.

    gst->terrain.num_max_visible_chunks = 8; // take in count error.
    for(size_t i = 0; i < gst->terrain.num_chunks; i++) {
        struct chunk_t* chunk = &gst->terrain.chunks[i];

        float dst = Vector3Length(chunk->center_pos);

        if(dst <= gst->render_dist) {
            gst->terrain.num_max_visible_chunks++;
        }
    }

    printf("'%s': New render distance: %0.3f\n", __func__, new_dist);
    printf("'%s': Predicted visible chunks: %i\n", __func__, gst->terrain.num_max_visible_chunks);


    // Allocate/Reallocate space for foliage.
    // When rendering terrain: foliage are copied into bigger arrays and then rendered all at once(per type)
    // ^ This will reduce number of draw calls.

    for(size_t i = 0; i < MAX_FOLIAGE_TYPES; i++) {
        struct foliage_rdata_t* f_rdata = &gst->terrain.foliage_rdata[i];

        if(f_rdata->matrices) {
            free(f_rdata->matrices);
            f_rdata->matrices = NULL;
        }

        f_rdata->matrices_size 
            = gst->terrain.num_max_visible_chunks
            * gst->terrain.foliage_max_perchunk[i];

        f_rdata->matrices = malloc(f_rdata->matrices_size * sizeof *f_rdata->matrices);
    
        printf("\033[35m(%p)\033[0m\n", f_rdata->matrices);
    }


    // Strech water plane to match render distance.
    gst->terrain.waterplane.transform = MatrixScale(gst->render_dist, 1.0, gst->render_dist);

    if(gst->fog.mode == FOG_MODE_TORENDERDIST) {
        set_fog_settings(gst, &gst->fog);
    }

}


