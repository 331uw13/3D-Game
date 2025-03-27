#ifndef GSTATE_H
#define GSTATE_H

#define GRAPHICS_API_OPENGL_43

#include <raylib.h>
#include <rcamera.h> // raylib camera

#include "light.h"
#include "player.h"
#include "psystem.h"
#include "terrain.h"
#include "enemy.h"
#include "item.h"
#include "gui.h"



// Enable: "Noclip", "Dev menu", "Render debug info"
#define DEV_MODE 1


#define DEF_SCRN_W 1500
#define DEF_SCRN_H 800

#define TARGET_FPS 500
#define CAMERA_SENSETIVITY 0.00125
#define MAX_VOLUME_DIST 720 // How far away can player hear sounds.?

#define GRAVITY_CONST 500
#define FONT_SPACING 1.0

#define FOG_MIN 1.0
#define FOG_MAX 10.0

// Index for 'textures'.
#define NONE_TEXID -1 // TODO: remove this <-
#define GRID4x4_TEXID 0
#define GRID6x6_TEXID 1
#define GRID9x9_TEXID 2
#define GUN_0_TEXID 3
#define ENEMY_LVL0_TEXID 4
#define PLAYER_ARMS_TEXID 5
#define CRITICALHIT_TEXID 6
#define TREEBARK_TEXID 7
#define LEAF_TEXID 8
#define ROCK_TEXID 9
#define MOSS_TEXID 10
#define GRASS_TEXID 11
#define GUNFX_TEXID 12
#define APPLE_INV_TEXID 13
#define APPLE_TEXID 14
#define METALPIECE_INV_TEXID 15
#define METALPIECE_TEXID 16
#define PLAYER_SKIN_TEXID 17
#define PLAYER_HANDS_TEXID 18
#define METAL2_TEXID 19
#define ENEMY_LVL1_TEXID 20
#define MAX_TEXTURES 21
// ...


// Index for 'sounds'
#define PLAYER_GUN_SOUND 0
#define ENEMY_HIT_SOUND_0 1
#define ENEMY_HIT_SOUND_1 2
#define ENEMY_HIT_SOUND_2 3
#define ENEMY_GUN_SOUND 4
#define PRJ_ENVHIT_SOUND 5
#define PLAYER_HIT_SOUND 6
#define ENEMY_EXPLOSION_SOUND 7
#define POWERUP_SOUND 8
#define MAX_SOUNDS 9
//...


// Index for 'shaders'
#define DEFAULT_SHADER 0
#define POSTPROCESS_SHADER 1
#define BLOOM_TRESHOLD_SHADER 2
#define WDEPTH_SHADER 3
#define WDEPTH_INSTANCE_SHADER 4
#define PRJ_ENVHIT_PSYS_SHADER 5
#define BASIC_WEAPON_PSYS_SHADER 6
#define FOLIAGE_SHADER 7
#define FOG_PARTICLE_SHADER 8
#define WATER_SHADER 9
#define GUNFX_SHADER 10
#define ENEMY_GUNFX_SHADER 11
#define CRYSTAL_FOLIAGE_SHADER 12
#define POWERUP_SHOP_BG_SHADER 13
#define EXPLOSION_PSYS_SHADER 14
#define PLAYER_HIT_SHADER 15
#define GBUFFER_SHADER 16
#define GBUFFER_INSTANCE_SHADER 17
#define MAX_SHADERS 18
// ...
 

// Player's weapon particle system is stored in player struct.
// Enemies have pointers into global particle systems and weapons.

// Global particle systems:
#define PLAYER_WEAPON_PSYS 0
#define ENEMY_WEAPON_PSYS 1
#define PROJECTILE_ENVHIT_PSYS 2
#define ENEMY_HIT_PSYS 3
#define FOG_EFFECT_PSYS 4
#define PLAYER_HIT_PSYS 5
#define EXPLOSION_PSYS 6
#define WATER_SPLASH_PSYS 7
#define ENEMY_GUNFX_PSYS 8
#define CLOUD_PSYS 9
#define PRJ_TRAIL_PSYS 10
#define MAX_PSYSTEMS 11
// ...


// TODO: Clean this up.
// Uniform locations for fragment shaders
// (index in 'fs_unilocs' array)
#define POSTPROCESS_TIME_FS_UNILOC 0
#define POSTPROCESS_SCREENSIZE_FS_UNILOC 1
#define POSTPROCESS_PLAYER_HEALTH_FS_UNILOC 2
#define POSTPROCESS_CAMTARGET_FS_UNILOC 3
#define POSTPROCESS_CAMPOS_FS_UNILOC 4
#define PROJECTILE_POSTPROCESS_SCREENSIZE_FS_UNILOC 5
#define FOLIAGE_SHADER_TIME_FS_UNILOC 6
#define WATER_SHADER_TIME_FS_UNILOC 7
#define GUNFX_SHADER_COLOR_FS_UNILOC 8
#define ENEMY_GUNFX_SHADER_COLOR_FS_UNILOC 9
#define CRYSTAL_FOLIAGE_SHADER_TIME_FS_UNILOC 10
#define MAX_FS_UNILOCS 16
// ...

#define NUM_BLOOMTRESH_FRAMES 16 // How many frames to save of 'bloom treshold'

// Normal lights:
#define SUN_NLIGHT 0
#define PLAYER_GUN_NLIGHT 1
// ...

// How many lights can decay at once?
#define MAX_DECAY_LIGHTS 16

#define LIGHT_UB_STRUCT_SIZE (4*4 + 4*4 + 4*4 + 4*4)
#define FOG_UB_STRUCT_SIZE (4*4 + 4*4 + 4*4)


// Uniform buffer objects.
#define FOG_UBO 0
#define LIGHTS_UBO 1     // "Normal" lights
#define PRJLIGHTS_UBO 2  // Projectile lights.
#define MAX_UBOS 3


#define ENEMY_LVL0_WEAPON 0
#define ENEMY_LVL1_WEAPON 1
#define MAX_ENEMY_WEAPONS 2


#define MAX_ALL_ENEMIES 64 // Total max enemies.
#define MAX_ENEMY_MODELS 2



//#define MAX_RENDER_CRITHITS 8

// Static lights in lights ubo
#define SUN_LIGHT_ID 0
#define PLAYER_GUN_LIGHT_ID 1
#define MAX_STATIC_LIGHTS 2

// Dynamic lights in lights ubo
#define EXPLOSION_LIGHTS_ID 3
#define MAX_EXPLOSION_LIGHTS (MAX_NORMAL_LIGHTS - MAX_STATIC_LIGHTS)



#define SSAO_KERNEL_SIZE 64


// Critical hit marker.
struct crithit_marker_t {
    Vector3 position;
    float   lifetime; // Scale smaller overtime.
    int     visible;
    float   dst; // Distance to player. Used for sorting to fix alpha blending.
};


struct gbuffer_t {
    unsigned int normal_tex;
    unsigned int position_tex;
    unsigned int difspec_tex;// <- NOTE: this is ignored for now.

    unsigned int framebuffer;
    unsigned int depthbuffer;

};

// Game state "gst".
struct state_t {
    float time;
    float dt; // Previous frame time.
    struct player_t player;
    Font font;

    unsigned int ubo[MAX_UBOS];

    float fog_density;
    Color fog_color_near;
    Color fog_color_far;

    size_t num_prj_lights;


    Shader shaders[MAX_SHADERS];
    int    fs_unilocs[MAX_FS_UNILOCS];

    Texture       textures[MAX_TEXTURES];
    unsigned int  num_textures;

    // Light can be added to this array to be decayed/dimmed over time before disabling them completely.
    struct light_t decay_lights[MAX_DECAY_LIGHTS];
    size_t next_explosion_light_index;

    struct psystem_t psystems[MAX_PSYSTEMS];
    struct terrain_t terrain;

    //struct spawn_system_t spawnsys;
    struct ent_spawnsys_t enemy_spawn_systems[MAX_ENEMY_TYPES];
    Model  enemy_models[MAX_ENEMY_MODELS];
    struct enemy_t enemies[MAX_ALL_ENEMIES];
    size_t num_enemies;
   

    struct weapon_t enemy_weapons[MAX_ENEMY_WEAPONS];
    size_t num_enemy_weapons;


    struct item_t items[MAX_ALL_ITEMS];
    Model         item_models[MAX_ITEM_MODELS];
    size_t        num_items;

    float         natural_item_spawn_timers[MAX_ITEM_TYPES];

    int scrn_w; // Screen width
    int scrn_h; // Screen height

    int rseed; // Seed for randomgen functions.
    int debug;

    /*
    struct crithit_marker_t crithit_markers[MAX_RENDER_CRITHITS];
    size_t num_crithit_markers;
    float  crithit_marker_maxlifetime;
    */

    int xp_value_add;
    float xp_update_timer;


    int has_audio;
    Sound sounds[MAX_SOUNDS];

    struct gbuffer_t gbuffer;
   
    // Everything is rendered to this texture
    // and then post processed.
    RenderTexture2D env_render_target;

    // Bloom treshold is written here.
    // when post processing. bloom is aplied and mixed into 'env_render_target' texture
    RenderTexture2D bloomtresh_target;


    Matrix cam_view_matrix;
    Matrix cam_proj_matrix;
    Texture ssao_noise_tex;
    Vector3 ssao_kernel[SSAO_KERNEL_SIZE];

    // (NOT CURRENTLY USED)
    RenderTexture2D depth_texture;


    Color render_bg_color;
    int running;
    int menu_open;
    int devmenu_open;

};


void state_setup_gbuffer(struct state_t* gst);
void state_setup_ssao(struct state_t* gst);
void state_delete_gbuffer(struct state_t* gst);
void state_create_ubo(struct state_t* gst, int ubo_index, int binding_point, size_t size);

void state_setup_render_targets(struct state_t* gst);
void state_update_shader_uniforms(struct state_t* gst);
void state_update_frame(struct state_t* gst);

void update_fog_settings(struct state_t* gst);
void create_explosion(struct state_t* gst, Vector3 position, float damage, float radius);


// Initialization.
void state_setup_all_shaders(struct state_t* gst);
void state_setup_all_weapons(struct state_t* gst);
void state_setup_all_psystems(struct state_t* gst);
void state_setup_all_textures(struct state_t* gst);
void state_setup_all_sounds(struct state_t* gst);
void state_setup_all_enemy_models(struct state_t* gst);
void state_setup_all_item_models(struct state_t* gst);

// Free up memory.
void state_delete_all_shaders(struct state_t* gst);
void state_delete_all_psystems(struct state_t* gst);
void state_delete_all_sounds(struct state_t* gst);
void state_delete_all_textures(struct state_t* gst);
void state_delete_all_enemy_models(struct state_t* gst);
void state_delete_all_item_models(struct state_t* gst);


// Misc.
//void state_add_crithit_marker(struct state_t* gst, Vector3 position);



#endif
