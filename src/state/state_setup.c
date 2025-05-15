#include <stdio.h>
#include <stdlib.h>

#include "state.h"
#include "state_setup.h"
#include "../util.h"

#include <raylib.h>
#include <rlgl.h>

#include "../particle_systems/weapon_psys.h"
#include "../particle_systems/projectile_envhit_psys.h"
#include "../particle_systems/enemy_hit_psys.h"
#include "../particle_systems/fog_effect_psys.h"
#include "../particle_systems/player_hit_psys.h"
#include "../particle_systems/explosion_psys.h"
#include "../particle_systems/water_splash_psys.h"
#include "../particle_systems/enemy_gunfx_psys.h"
#include "../particle_systems/cloud_psys.h"
#include "../particle_systems/prj_trail_psys.h"
#include "../terrain.h"


#define PRINT_CURRENT_SETUP printf("\033[96m/ \033[96m%s\033[0m\n", __func__)
#define PRINT_CURRENT_SETUP_DONE printf("\033[36m\\---> Done.\033[0m\n")


static void state_setup_all_enemy_models(struct state_t* gst) {
    PRINT_CURRENT_SETUP;

    for(size_t i = 0; i < MAX_ALL_ENEMIES; i++) {
        gst->enemies[i].alive = 0;
    }

    load_enemy_model(gst, ENEMY_LVL0, "res/models/enemy_lvl0.glb", ENEMY_LVL0_TEXID);
    load_enemy_model(gst, ENEMY_LVL1, "res/models/enemy_lvl1.glb", ENEMY_LVL1_TEXID);

    gst->init_flags |= INITFLG_ENEMY_MODELS;
    PRINT_CURRENT_SETUP_DONE;
}



static int state_setup_all_sounds(struct state_t* gst) {
    PRINT_CURRENT_SETUP;
    int result = 0;
    SetTraceLogLevel(LOG_ALL);
    gst->has_audio = 0;
    InitAudioDevice();

    if(!IsAudioDeviceReady()) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Failed to initialize audio device\033[0m\n",
                __func__);
        goto error;
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
    gst->sounds[CLOUDBURST_SOUND] = LoadSound("res/audio/cloudburst.wav");
    

    SetMasterVolume(100.0);
    gst->has_audio = 1;
    result = 1;

    gst->init_flags |= INITFLG_SOUNDS;

error:
    SetTraceLogLevel(LOG_NONE);
    PRINT_CURRENT_SETUP_DONE;
    return result;
}


static void load_colorpick_texture(struct state_t* gst) {
    int width = 200;
    int height = 200;

    Color* pixels = malloc((width*height) * sizeof *pixels);


    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            Vector2 pos = (Vector2){ x, y };
            float dist = pos.x / (float)width;
            float damp = (float)y/(float)(height);
            damp = CLAMP(damp, 0.0, 1.0);
            size_t index = y * width + x;
            pixels[index] = ColorFromHSV(dist * 360, 1.0, damp);
        }
    }

    gst->colorpick_img = (Image) {
        .data = pixels,
        .width = width,
        .height = height,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };

    gst->colorpick_tex = LoadTextureFromImage(gst->colorpick_img);

}

static int load_texture(struct state_t* gst, const char* filepath, int texid) {
    int result = 0;
    if(texid >= MAX_TEXTURES) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Texture id is invalid. (%i) for '%s' (out of bounds?)\033[0m\n",
                __func__, texid, filepath);
        goto error;
    }

    if(!FileExists(filepath)) {
        fprintf(stderr, "\033[31m(ERROR) '%s': \"%s\" Doesnt exists\033[0m\n",
                __func__, filepath);
        goto error;
    }

    gst->textures[texid] = LoadTexture(filepath);
    gst->num_textures++;

    result = 1;
error:
    return result;
}

static void state_setup_all_textures(struct state_t* gst) {
    PRINT_CURRENT_SETUP;
    SetTraceLogLevel(LOG_ALL);

    load_texture(gst, "res/textures/grid_4x4.png", GRID4x4_TEXID);
    load_texture(gst, "res/textures/grid_6x6.png", GRID6x6_TEXID);
    load_texture(gst, "res/textures/grid_9x9.png", GRID9x9_TEXID);
    load_texture(gst, "res/textures/gun_0.png", GUN_0_TEXID);
    load_texture(gst, "res/textures/enemy_lvl0.png", ENEMY_LVL0_TEXID);
    load_texture(gst, "res/textures/arms.png", PLAYER_ARMS_TEXID);
    load_texture(gst, "res/textures/tree_bark.png", TREEBARK_TEXID);
    load_texture(gst, "res/textures/leaf.jpg", LEAF_TEXID);
    load_texture(gst, "res/textures/rock_type0.jpg", ROCK_TEXID);
    load_texture(gst, "res/textures/moss2.png", TERRAIN_TEXID);
    load_texture(gst, "res/textures/apple.png", APPLE_TEXID);
    load_texture(gst, "res/textures/cloth.png", PLAYER_HANDS_TEXID);
    load_texture(gst, "res/textures/player_skin.png", PLAYER_SKIN_TEXID);
    load_texture(gst, "res/textures/enemy_lvl1.png", ENEMY_LVL1_TEXID);
    load_texture(gst, "res/textures/mushroom_body.png", MUSHROOM_BODY_TEXID);
    load_texture(gst, "res/textures/mushroom_hat.png", MUSHROOM_HAT_TEXID);
    load_texture(gst, "res/textures/terrain_mushroom.png", TERRAIN_MUSHROOM_TEXID);
    load_texture(gst, "res/textures/hazy_biome_ground.jpg", HAZYBIOME_GROUND_TEXID);
    load_texture(gst, "res/textures/evil_biome_ground.png", EVILBIOME_GROUND_TEXID);
    

    SetTraceLogLevel(LOG_NONE); 

    load_colorpick_texture(gst);
   
    SetTextureWrap(gst->textures[TREEBARK_TEXID], TEXTURE_WRAP_MIRROR_REPEAT);
    SetTextureWrap(gst->textures[LEAF_TEXID], TEXTURE_WRAP_MIRROR_REPEAT);
    SetTextureWrap(gst->textures[ROCK_TEXID], TEXTURE_WRAP_MIRROR_REPEAT);
    SetTextureWrap(gst->textures[TERRAIN_TEXID], TEXTURE_WRAP_MIRROR_REPEAT);
    SetTextureWrap(gst->textures[HAZYBIOME_GROUND_TEXID], TEXTURE_WRAP_MIRROR_REPEAT);
    SetTextureWrap(gst->textures[EVILBIOME_GROUND_TEXID], TEXTURE_WRAP_MIRROR_REPEAT);

    gst->init_flags |= INITFLG_TEXTURES;
    PRINT_CURRENT_SETUP_DONE;
}



static void state_setup_all_weapons(struct state_t* gst) {

   
    // Player's weapon.
    gst->player.weapon = (struct weapon_t) {
        .gid = PLAYER_WEAPON_GID,
        .accuracy = 8.35,
        .damage = 10.0,
        .critical_chance = 10,
        .critical_mult = 1.85,
        .prj_speed = 500.0,
        .prj_max_lifetime = 15.0,
        .prj_hitbox_size = (Vector3) { 1.0, 1.0, 1.0 },
        .prj_scale = 1.0,
        .color = (Color) { 20, 255, 200, 255 },
        
        // TODO -----
        .overheat_temp = 100.0,
        .heat_increase = 2.0,
        .cooling_level = 20.0
    };

    init_weapon_lqmag(gst, &gst->player.weapon, LQMAG_TYPE_SMALL);
 

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
        .prj_scale = 2.0,
        .color = (Color){ 255, 20, 120, 200 },
        .overheat_temp = -1,
        .lqmag = (struct lqmag_t) { .infinite = 1 }
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
        .lqmag = (struct lqmag_t) { .infinite = 1 }
    };
}

static void state_setup_all_psystems(struct state_t* gst) {

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

        psystem->particle_model = LoadModelFromMesh(GenMeshSphere(1.25, 16, 16));
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

        psystem->particle_model = LoadModelFromMesh(GenMeshSphere(1.5, 16, 16));
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

        psystem->particle_model = LoadModelFromMesh(GenMeshSphere(0.5, 32, 32));
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

        psystem->particle_model = LoadModelFromMesh(GenMeshSphere(1.25, 16, 16));
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

        psystem->particle_model = LoadModelFromMesh(GenMeshSphere(0.8, 8, 8));
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

        psystem->particle_model = LoadModelFromMesh(GenMeshSphere(0.75, 8, 8));
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

        psystem->particle_model = LoadModelFromMesh(GenMeshSphere(0.5, 4, 4));
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

        psystem->particle_model = LoadModelFromMesh(GenMeshSphere(0.45, 8, 8));
        psystem->userptr = &gst->player.weapon;
        setup_psystem_color_vbo(gst, psystem);
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

        psystem->particle_model = LoadModelFromMesh(GenMeshSphere(0.6, 8, 8));
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
        psystem->particle_model = LoadModelFromMesh(GenMeshPlane(1.0, 1.0, 1, 1));
        setup_psystem_color_vbo(gst, psystem);
    }


    // Create CLOUD_PSYS.
    {
        const size_t num_cloud_parts = 400;
        struct psystem_t* psystem = &gst->psystems[CLOUD_PSYS];
        create_psystem(
                gst,
                PSYS_GROUPID_ENV,
                PSYS_CONTINUOUS,
                psystem,
                num_cloud_parts,
                cloud_psys_update,
                cloud_psys_init,
                CLOUD_PARTICLE_SHADER
                );

        psystem->particle_model = LoadModel("res/models/cloud.glb");
        add_particles(gst, psystem, num_cloud_parts, 
                (Vector3){0}, (Vector3){0}, (Color){0},
                NULL, NO_EXTRADATA, NO_IDB);

    }

    gst->init_flags |= INITFLG_PSYSTEMS;
}




static void state_setup_all_shaders(struct state_t* gst) {
    PRINT_CURRENT_SETUP;
    SetTraceLogLevel(LOG_ALL);

    // --- Setup Default Shader ---
    {
        Shader* shader = &gst->shaders[DEFAULT_SHADER];
        load_shader(gst,
            "res/shaders/default.vs",
            "res/shaders/default.fs",
            NO_GEOMETRY_SHADER,
            shader);
        
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(*shader, "matModel");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
    }


    // --- Setup Post Processing Shader ---
    {
        Shader* shader = &gst->shaders[POSTPROCESS_SHADER];
        load_shader(gst,
            "res/shaders/default.vs",
            "res/shaders/postprocess.fs",
            NO_GEOMETRY_SHADER,
            shader);
    }

    // --- PRJ_ENVHIT_PSYS_SHADER ---
    {
        Shader* shader = &gst->shaders[PRJ_ENVHIT_PSYS_SHADER];
        load_shader(gst,
            "res/shaders/instance_core.vs",
            "res/shaders/prj_envhit_psys.fs",
            NO_GEOMETRY_SHADER,
            shader);
       
        shader->locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(*shader, "mvp");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
    }
 

    // --- PLAYER_WEAPON_PSYS_SHADER ---
    {
        Shader* shader = &gst->shaders[BASIC_WEAPON_PSYS_SHADER];
        load_shader(gst,
            "res/shaders/instance_core.vs",
            "res/shaders/basic_weapon_psys.fs",
            NO_GEOMETRY_SHADER,
            shader);
       
        shader->locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(*shader, "mvp");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
    }
 


    // --- EXPLOSION_PSYS_SHADER ---
    {
        Shader* shader = &gst->shaders[EXPLOSION_PSYS_SHADER];
        load_shader(gst,
            "res/shaders/instance_core.vs",
            "res/shaders/explosion_psys.fs",
            NO_GEOMETRY_SHADER,
            shader);
       
        shader->locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(*shader, "mvp");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
    }


    // --- FOLIAGE_SHADER ---
    {
        Shader* shader = &gst->shaders[FOLIAGE_SHADER];
        load_shader(gst,
            "res/shaders/foliage.vs",
            "res/shaders/foliage.fs",
            NO_GEOMETRY_SHADER,
            shader);
       
        shader->locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(*shader, "mvp");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
    }

    // --- FOLIAGE_WIND_SHADER ---
    {
        Shader* shader = &gst->shaders[FOLIAGE_WIND_SHADER];
        load_shader(gst,
            "res/shaders/foliage_wind.vs",
            "res/shaders/foliage.fs",
            NO_GEOMETRY_SHADER,
            shader);
       
        shader->locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(*shader, "mvp");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
    }


    // --- FOG_EFFECT_PARTICLE_SHADER ---
    {
        Shader* shader = &gst->shaders[FOG_PARTICLE_SHADER];
        load_shader(gst,
            "res/shaders/instance_core.vs",
            "res/shaders/fog_particle.fs",
            NO_GEOMETRY_SHADER,
            shader);
       
        shader->locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(*shader, "mvp");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
    }

    // --- CLOUD_PARTICLE_SHADER ---
    {
        Shader* shader = &gst->shaders[CLOUD_PARTICLE_SHADER];
        load_shader(gst,
            "res/shaders/cloud_particle.vs",
            "res/shaders/cloud_particle.fs",
            NO_GEOMETRY_SHADER,
            shader);
       
        shader->locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(*shader, "mvp");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
    }

    // --- WATER_SHADER ---
    {
        Shader* shader = &gst->shaders[WATER_SHADER];
        load_shader(gst,
            "res/shaders/default.vs",
            "res/shaders/water.fs",
            NO_GEOMETRY_SHADER,
            shader);
        
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(*shader, "matModel");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
    }

    // --- SKY_SHADER ---
    {
        Shader* shader = &gst->shaders[SKY_SHADER];
        load_shader(gst,
            "res/shaders/default.vs",
            "res/shaders/sky.fs",
            NO_GEOMETRY_SHADER,
            shader);
        
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(*shader, "matModel");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
    }

    // --- BLOOM_TRESHOLD_SHADER ---
    {
        Shader* shader = &gst->shaders[BLOOM_TRESHOLD_SHADER];
        load_shader(gst,
            "res/shaders/default.vs",
            "res/shaders/bloom_treshold.fs",
            NO_GEOMETRY_SHADER,
            shader);
    }

    // --- BLOOM_BLUR_SHADER ---
    {
        Shader* shader = &gst->shaders[BLOOM_BLUR_SHADER];
        load_shader(gst,
            "res/shaders/default.vs",
            "res/shaders/bloom_blur.fs",
            NO_GEOMETRY_SHADER,
            shader);
    }

    // --- GUNFX_SHADER (for player) ---
    {
        Shader* shader = &gst->shaders[GUNFX_SHADER];
        load_shader(gst,
            "res/shaders/default.vs",
            "res/shaders/player_gunfx.fs",
            NO_GEOMETRY_SHADER,
            shader);
    }

    // --- ENEMY_GUNFX_SHADER ---
    {
        Shader* shader = &gst->shaders[ENEMY_GUNFX_SHADER];
        load_shader(gst,
            "res/shaders/instance_core.vs",
            "res/shaders/enemy_gunfx.fs",
            NO_GEOMETRY_SHADER,
            shader);
       
        shader->locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(*shader, "mvp");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
    }

    // --- PLAYER_HIT_SHADER ---
    {
        Shader* shader = &gst->shaders[PLAYER_HIT_SHADER];
        load_shader(gst,
            "res/shaders/instance_core.vs",
            "res/shaders/player_hit.fs",
            NO_GEOMETRY_SHADER,
            shader);
       
        shader->locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(*shader, "mvp");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
    }


    // --- GBUFFER_SHADER ---
    {
        Shader* shader = &gst->shaders[GBUFFER_SHADER];
        load_shader(gst,
            "res/shaders/default.vs", 
            "res/shaders/gbuffer.fs",
            NO_GEOMETRY_SHADER,
            shader);
    }

    // --- GBUFFER_INSTANCE_SHADER ---
    {
        Shader* shader = &gst->shaders[GBUFFER_INSTANCE_SHADER];
        load_shader(gst,
            "res/shaders/instance_core.vs", 
            "res/shaders/gbuffer.fs",
            NO_GEOMETRY_SHADER,
            shader);

        shader->locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(*shader, "mvp");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
    }

    // --- GBUFFER_FOLIAGE_WIND_SHADER ---
    {
        Shader* shader = &gst->shaders[GBUFFER_FOLIAGE_WIND_SHADER];
        load_shader(gst,
            "res/shaders/foliage_wind.vs", 
            "res/shaders/gbuffer.fs",
            NO_GEOMETRY_SHADER,
            shader);

        shader->locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(*shader, "mvp");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
    }

    // --- SSAO_SHADER ---
    {
        Shader* shader = &gst->shaders[SSAO_SHADER];
        load_shader(gst,
            "res/shaders/default.vs", 
            "res/shaders/ssao.fs",
            NO_GEOMETRY_SHADER,
            shader);
    }

    // --- SSAO_BLUR_SHADER ---
    {
        Shader* shader = &gst->shaders[SSAO_BLUR_SHADER];
        load_shader(gst,
            "res/shaders/default.vs", 
            "res/shaders/ssao_blur.fs",
            NO_GEOMETRY_SHADER,
            shader);
    }

    // --- ENEGY_LIQUID_SHADER ---
    {
        Shader* shader = &gst->shaders[ENERGY_LIQUID_SHADER];
        load_shader(gst,
            "res/shaders/default.vs",
            "res/shaders/energy_liquid.fs",
            NO_GEOMETRY_SHADER,
            shader);
    }

    // --- FRACTAL_MODEL_SHADER ---
    {
        Shader* shader = &gst->shaders[FRACTAL_MODEL_SHADER];
        load_shader(gst,
            "res/shaders/default.vs",
            "res/shaders/fractal_model.fs",
            NO_GEOMETRY_SHADER,
            shader);
    }
    
    gst->init_flags |= INITFLG_SHADERS;
    SetTraceLogLevel(LOG_NONE);
    PRINT_CURRENT_SETUP_DONE;
}


#define GBUF_FLG_ALL       0x1
#define GBUF_FLG_POSITIONS 0x2
#define GBUF_FLG_NORMALS   0x4
#define GBUF_FLG_LDEPTH    0x8

static int state_setup_gbuffer(struct gbuffer_t* gbuf, int width, int height, int flags) {
    int result = 0;

    printf("\033[90m: Creating framebuffer %ix%i\n", width, height);
    SetTraceLogLevel(LOG_ALL);

    gbuf->framebuffer = rlLoadFramebuffer();
    if(!gbuf->framebuffer) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Failed to create framebuffer\033[0m\n",
                __func__);
        goto error;
    }

    gbuf->res_x = width;
    gbuf->res_y = height;

    rlEnableFramebuffer(gbuf->framebuffer);

    // ------- Create Textures.

    int num_drawbuffers = 0;

    // Positions.
    if((flags & GBUF_FLG_ALL) || (flags & GBUF_FLG_POSITIONS)) {
        gbuf->position_tex = rlLoadTexture(NULL, width, height, RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32, 1);
        num_drawbuffers++;
    }

    // Normals.
    if((flags & GBUF_FLG_ALL) || (flags & GBUF_FLG_NORMALS)) {
        gbuf->normal_tex = rlLoadTexture(NULL, width, height, RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32, 1);
        num_drawbuffers++;
    }
    
    // Linear Depth.
    if((flags & GBUF_FLG_ALL) || (flags & GBUF_FLG_LDEPTH)) {
        gbuf->depth_tex = rlLoadTexture(NULL, width, height, RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32, 1);
        num_drawbuffers++;
    }
   
    gbuf->depthbuffer =  rlLoadTextureDepth(width, height, 1);

    rlActiveDrawBuffers(num_drawbuffers);

    // -------- Attach textures to framebuffer.

    // Positions.
    if((flags & GBUF_FLG_ALL) || (flags & GBUF_FLG_POSITIONS)) {
        rlFramebufferAttach(gbuf->framebuffer, gbuf->position_tex,
                RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
        printf("\033[90m: Attached texture for positions.\033[0m\n");
    }
    // Normals.
    if((flags & GBUF_FLG_ALL) || (flags & GBUF_FLG_NORMALS)) {
        rlFramebufferAttach(gbuf->framebuffer, gbuf->normal_tex,
             RL_ATTACHMENT_COLOR_CHANNEL1, RL_ATTACHMENT_TEXTURE2D, 0);
        printf("\033[90m: Attached texture for normals.\033[0m\n");
    }
    // Linear Depth.
    if((flags & GBUF_FLG_ALL) || (flags & GBUF_FLG_LDEPTH)) {
        rlFramebufferAttach(gbuf->framebuffer, gbuf->depth_tex,
                RL_ATTACHMENT_COLOR_CHANNEL2, RL_ATTACHMENT_TEXTURE2D, 0);
        printf("\033[90m: Attached texture for linear depth.\033[0m\n");
    }

    // Attach depth buffer.
    rlFramebufferAttach(gbuf->framebuffer, gbuf->depthbuffer,
            RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);

    // This function also will disable the framebuffer.
    if(!rlFramebufferComplete(gbuf->framebuffer)) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Framebuffer is not complete!\033[0m\n",
                __func__);
        goto error;
    }
 
    printf("\033[90m: Framebuffer complete.\033[0m\n");
    result = 1;

error:
    SetTraceLogLevel(LOG_NONE);
    return result;
}



static void state_setup_all_gbuffers(struct state_t* gst) {
    PRINT_CURRENT_SETUP;


    state_setup_gbuffer(&gst->gbuffer, 
            gst->ssao_res_x, gst->ssao_res_y,
            GBUF_FLG_ALL);
    
    gst->shadow_res_x = gst->res_x;
    gst->shadow_res_y = gst->res_y;

    for(int i = 0; i < MAX_SHADOW_LEVELS; i++) {
        if(!state_setup_gbuffer(&gst->shadow_gbuffers[i],
                gst->shadow_res_x, gst->shadow_res_y,
                GBUF_FLG_POSITIONS)) {
            state_abort(gst);
        }
    }

    gst->init_flags |= INITFLG_GBUFFERS;


    PRINT_CURRENT_SETUP_DONE;
}

static void state_setup_all_render_targets(struct state_t* gst) {
    PRINT_CURRENT_SETUP;
    SetTraceLogLevel(LOG_ALL);


    int dres_x = gst->res_x / gst->cfg.res_div;
    int dres_y = gst->res_y / gst->cfg.res_div;
    
    gst->env_render_target = LoadRenderTexture(dres_x, dres_y);
    gst->inv_render_target = LoadRenderTexture(dres_x, dres_y);
    gst->bloomtresh_target = LoadRenderTexture(dres_x, dres_y);

    gst->env_render_downsample = LoadRenderTexture(gst->ssao_res_x, gst->ssao_res_y);

    gst->ssao_final = LoadRenderTexture(
            gst->res_x,
            gst->res_y
            );


    gst->ssao_target = LoadRenderTexture(gst->ssao_res_x, gst->ssao_res_y);

    SetTextureFilter(gst->env_render_downsample.texture, TEXTURE_FILTER_BILINEAR);

    gst->bloom_downsamples[0] = LoadRenderTexture(909, 485);    
    gst->bloom_downsamples[1] = LoadRenderTexture(638, 340);


    SetTextureFilter(gst->bloom_downsamples[0].texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(gst->bloom_downsamples[1].texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(gst->env_render_downsample.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(gst->env_render_downsample.texture, TEXTURE_FILTER_BILINEAR);

    gst->init_flags |= INITFLG_RENDERTARGETS;

    SetTraceLogLevel(LOG_NONE);
    PRINT_CURRENT_SETUP_DONE;
}

static void state_setup_all_ubos(struct state_t* gst) {
    state_create_ubo(gst, LIGHTS_UBO,    2, MAX_NORMAL_LIGHTS * LIGHT_UB_STRUCT_SIZE);
    state_create_ubo(gst, PRJLIGHTS_UBO, 3, MAX_PROJECTILE_LIGHTS * LIGHT_UB_STRUCT_SIZE);
    state_create_ubo(gst, FOG_UBO,       4, FOG_UB_STRUCT_SIZE);

    gst->init_flags |= INITFLG_UBOS;
}


static void state_setup_ssao(struct state_t* gst) {
    
    gst->ssao_kernel = NULL;
    gst->ssao_kernel = malloc(gst->cfg.ssao_kernel_samples * sizeof *gst->ssao_kernel);

    for(int i = 0; i < gst->cfg.ssao_kernel_samples; i++) {

        // Get scale factor to move points closer to the center.
        float scale = (float)i / (float)gst->cfg.ssao_kernel_samples;
        scale = lerp(scale*scale, 0.1, 1.0);

        Vector3 sample = (Vector3) {
            RSEEDRANDOMF(-1.0, 1.0) * scale,
            RSEEDRANDOMF(-1.0, 1.0) * scale,
            RSEEDRANDOMF(-1.0, 1.0) * scale
        };
  

        gst->ssao_kernel[i] = sample;
    }

    int width = 1024;
    int height = 1024;
    Color *pixels = (Color *)malloc(width*height*sizeof(Color));

    for (int i = 0; i < width*height; i++) {
        pixels[i] = (Color) {
            GetRandomValue(0, 255)*0.25,
            GetRandomValue(0, 255)*0.25,
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
 
    gst->ssao_res_x = 0;
    gst->ssao_res_y = 0;

    switch(gst->cfg.ssao_quality) {
        default:
        case CFG_SSAO_QLOW:
            gst->ssao_res_x = gst->res_x/2.0;
            gst->ssao_res_y = gst->res_y/2.0;
            break;

        case CFG_SSAO_QMED:
            gst->ssao_res_x = gst->res_x/1.35;
            gst->ssao_res_y = gst->res_y/1.35;
            break;

        case CFG_SSAO_QHIGH:
            gst->ssao_res_x = gst->res_x;
            gst->ssao_res_y = gst->res_y;
            break;
    } 

    gst->init_flags |= INITFLG_SSAO;
}

static void state_setup_shadow_cams(struct state_t* gst) {


    float fovs[MAX_SHADOW_LEVELS] = {
        180.0,
        720.0,
        1000.0
    };

    for(int i = 0; i < MAX_SHADOW_LEVELS; i++) {
        Camera* cam = &gst->shadow_cams[i];
        *cam = (Camera){ 0 };
        cam->position = (Vector3){0};
        cam->target = (Vector3){0};
        cam->up = (Vector3){ 0.0, 1.0, 0.0 };
        cam->fovy = fovs[i];
        cam->projection = CAMERA_ORTHOGRAPHIC;

        printf("\033[90m: Created shadow camera with fov %0.4f\033[0m\n", fovs[i]);
    }
  
    gst->shadow_cam_height = 300.0;
}

static void state_setup_terrain(struct state_t* gst) {
    const float terrain_scale = 30.0;
    const u32   terrain_size = 2048;
    const float terrain_amplitude = 50.0;
    const float terrain_pnfrequency = 70.0;
    const int   terrain_octaves = 3;
    
    init_perlin_noise();
    gst->terrain = (struct terrain_t) { 0 };
    setup_biomes(gst);

    //const int terrain_seed = GetRandomValue(0, 9999999);
    const int terrain_seed = 4685102;

    printf("'%s': Terrain seed = %i\n",
            __func__, terrain_seed);

    generate_terrain(
            gst, &gst->terrain,
            terrain_size,
            terrain_scale,
            terrain_amplitude,
            terrain_pnfrequency,
            terrain_octaves,
            terrain_seed
            );

    gst->init_flags |= INITFLG_TERRAIN;
}


static void state_setup_all_item_models(struct state_t* gst) {

    if(!load_item_model(gst, ITEM_APPLE, APPLE_TEXID, "res/models/apple.glb"))
    { state_abort(gst); }
    add_item_namedesc(gst, ITEM_APPLE, "Apple", "Healthy food.");
    



    gst->init_flags |= INITFLG_ITEM_MODELS;
}



int state_setup_everything(struct state_t* gst) {
    int result = 0;

    gst->init_flags = 0;
    // TODO: Is this used?
    gst->ssao_res_x = 0;
    gst->ssao_res_y = 0;
    

    state_setup_all_textures(gst);
    state_setup_all_shaders(gst);
    state_setup_all_ubos(gst);

    // Make sure all lights are disabled.
    for(int i = 0; i < MAX_NORMAL_LIGHTS; i++) {
        struct light_t disabled = {
            .enabled = 0,
            .index = i
        };
        set_light(gst, &disabled, LIGHTS_UBO);
    }
    for(size_t i = 0; i < MAX_DECAY_LIGHTS; i++) {
        gst->decay_lights[i].enabled = 0;
    }

    gst->weather.wind_dir = (Vector3){ 0, 0, 1 };
    gst->weather.wind_strength = 100.0;
    //gst->weather.sun_color = (Color){ 255, 140, 30, 255 };


    state_setup_terrain(gst);
    state_setup_ssao(gst);
    
    state_setup_all_psystems(gst);
    state_setup_all_weapons(gst);
    state_setup_all_sounds(gst);
    state_setup_all_item_models(gst);
    state_setup_all_enemy_models(gst);

    state_setup_all_gbuffers(gst);
    state_setup_all_render_targets(gst);

    init_player_struct(gst, &gst->player);
    change_to_biome(gst, get_biomeid_by_ylevel(gst, gst->player.position.y));
    
    state_setup_shadow_cams(gst);
    set_render_dist(gst, gst->cfg.render_dist);
    
    // FOR TEST
    {
        gst->fractal_rx = 0.3;
        gst->fractal_ry = 0.2;
        gst->fractal_rz = 0.0;

        fractalgen_tree(gst, &gst->test_fractal);

    }



    gst->show_only_ssao = 0;
    shader_setu_int(gst, POSTPROCESS_SHADER, U_ONLY_SSAO, &gst->show_only_ssao);

    shader_setu_float(gst, SSAO_SHADER, U_RES_DIV, &gst->cfg.res_div);

    // For inventories
    gst->inventory_box_model = LoadModel("res/models/inventory_box.glb");
    gst->inventory_box_model.materials[0] = LoadMaterialDefault();
    gst->inventory_box_model.materials[0].shader = gst->shaders[DEFAULT_SHADER];

    gst->energy_liquid_material = LoadMaterialDefault();
    gst->energy_liquid_material.shader = gst->shaders[ENERGY_LIQUID_SHADER];

    printf("\033[32m'%s': Done\033[0m\n", __func__);
    result = 1;
    return result;
}


