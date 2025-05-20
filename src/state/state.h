#ifndef GSTATE_H
#define GSTATE_H

#define GRAPHICS_API_OPENGL_43

#include <raylib.h>
#include <rcamera.h> // raylib camera

#include "../light.h"
#include "../player.h"
#include "../psystem.h"
#include "../terrain.h"
#include "../enemy.h"
#include "../item.h"
#include "../gui.h"
#include "../shader_util.h"
#include "../npc.h"
#include "../config.h"
#include "../fog.h"
#include "../item.h"
#include "../fractalgen.h"
#include "../weapon_model.h"

// Enable: "Noclip", "Dev menu", "Render debug info"
#define DEV_MODE 1


#define DEFAULT_RES_X 1500  // If resolution is not found from config file
#define DEFAULT_RES_Y 800   // use default resolution.
#define DEFAULT_SSAO_KERNEL_SAMPLES 32

#define RESOLUTION_X 1500
#define RESOLUTION_Y 800
#define FONT_SPACING 1.0

#define MAX_RENDERDIST 15000.0
#define MIN_RENDERDIST 2200.0

#define TARGET_FPS 500
#define CAMERA_SENSETIVITY 0.00125
#define MAX_VOLUME_DIST 3000 // How far away can player hear sounds.?

#define GRAVITY_CONST 500

#define FOG_MIN 1.0
#define FOG_MAX 10.0

// Index for 'textures'.
#define GRID4x4_TEXID 0
#define GRID6x6_TEXID 1
#define GRID9x9_TEXID 2
#define GUN_0_TEXID 3
#define ENEMY_LVL0_TEXID 4
#define PLAYER_ARMS_TEXID 5
#define TREEBARK_TEXID 6
#define LEAF_TEXID 7
#define ROCK_TEXID 8
#define TERRAIN_TEXID 9
#define APPLE_TEXID 10
#define PLAYER_SKIN_TEXID 11
#define PLAYER_HANDS_TEXID 12
#define ENEMY_LVL1_TEXID 12
#define MUSHROOM_HAT_TEXID 13
#define MUSHROOM_BODY_TEXID 14
#define TERRAIN_MUSHROOM_TEXID 15
#define HAZYBIOME_GROUND_TEXID 16
#define EVILBIOME_GROUND_TEXID 17
#define MAX_TEXTURES 18
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
#define CLOUDBURST_SOUND 9
#define MAX_SOUNDS 10
//...


// Index for 'shaders'   | TODO: Delete unused shaders.
#define DEFAULT_SHADER 0
#define POSTPROCESS_SHADER 1
#define BLOOM_TRESHOLD_SHADER 2
#define PRJ_ENVHIT_PSYS_SHADER 3
#define BASIC_WEAPON_PSYS_SHADER 4
#define FOLIAGE_SHADER 5
#define FOG_PARTICLE_SHADER 6
#define WATER_SHADER 7
#define GUNFX_SHADER 8
#define ENEMY_GUNFX_SHADER 9
#define EXPLOSION_PSYS_SHADER 10
#define PLAYER_HIT_SHADER 11
#define GBUFFER_SHADER 12
#define GBUFFER_INSTANCE_SHADER 13
#define GBUFFER_FOLIAGE_WIND_SHADER 14
#define SSAO_SHADER 15
#define BLOOM_BLUR_SHADER 16
#define SSAO_BLUR_SHADER 17
#define SKY_SHADER 18
#define CLOUD_PARTICLE_SHADER 19
#define FOLIAGE_WIND_SHADER 20
#define ENERGY_LIQUID_SHADER 24
#define FRACTAL_MODEL_SHADER 25
#define MAX_SHADERS 26
// ...


// Particle systems:
#define PLAYER_WEAPON_PSYS 0
#define ENEMY_WEAPON_PSYS 1
#define PROJECTILE_ENVHIT_PSYS 2
#define ENEMY_HIT_PSYS 3
#define FOG_EFFECT_PSYS 4
#define PLAYER_HIT_PSYS 5
#define EXPLOSION_PSYS 6
#define ENEMY_GUNFX_PSYS 7
#define CLOUD_PSYS 8
#define PRJ_TRAIL_PSYS 9
#define MAX_PSYSTEMS 10
// ...


// Normal lights:
#define SUN_NLIGHT 0
#define PLAYER_GUN_NLIGHT 1
// ...

// Uniform buffer objects structs sizes.
#define LIGHT_UB_STRUCT_SIZE        (4*4 + 4*4 + 4*4 + 4*4)
#define FOG_UB_STRUCT_SIZE          (4*4 + 4*4 + 4*4)
//#define GRASS_FVEC_UB_STRUCT_SIZE   (4*4)

//#define MAX_GRASS_FORCEVECTORS 64 // NOTE: This must be same as in 'res/shaders/grass/grass.cs'
//#define MAX_PRJ_FORCEVECTORS 32 // Rest are reserved for something else.

// Uniform buffer objects.
#define FOG_UBO 0
#define LIGHTS_UBO 1     // "Normal" lights
#define PRJLIGHTS_UBO 2  // Projectile lights.
//#define FORCEVEC_UBO 3   // Used for grass.
#define MAX_UBOS 4

// Shader storage buffer objects.
#define GRASSDATA_SSBO 0
#define MAX_SSBOS 1


#define NUM_BLOOM_DOWNSAMPLES 2

// Dynamic lights in lights ubo
#define EXPLOSION_LIGHTS_ID 3
#define MAX_EXPLOSION_LIGHTS (MAX_NORMAL_LIGHTS - MAX_STATIC_LIGHTS)



#define MAX_SSAO_KERNEL_SIZE 128 // IMPORTANT NOTE: This must be same as in 'res/shaders/ssao.fs'
#define MAX_SHADOW_LEVELS 3

// Used to know what has been succesfully initialized.
#define INITFLG_TERRAIN       (1<<0)
#define INITFLG_ENEMY_MODELS  (1<<1)
#define INITFLG_ITEM_MODELS   (1<<2)
#define INITFLG_WEAPON_MODELS (1<<3)
#define INITFLG_PSYSTEMS      (1<<4)
#define INITFLG_SHADERS       (1<<5)
#define INITFLG_UBOS          (1<<6)
#define INITFLG_RENDERTARGETS (1<<7)
#define INITFLG_GBUFFERS      (1<<8)
#define INITFLG_SOUNDS        (1<<9)
#define INITFLG_TEXTURES      (1<<10)
#define INITFLG_SSAO          (1<<11)
#define INITFLG_PLAYER        (1<<12)
#define INITFLG_NPC           (1<<13)


// 'Time buffer' is for saving render times and processing times
// So the average can be calculated.
// How many to record?
// If the 'TIME_ELEM_<something>' ends with _R it means its for rendering.
#define TIMEBUF_SIZE 24
#define TIMEBUF_ELEM_PSYSTEMS_R 0
#define TIMEBUF_ELEM_TERRAIN_R 1
#define TIMEBUF_MAX_ELEMS 2

/*
// Critical hit marker.
struct crithit_marker_t {
    Vector3 position;
    float   lifetime; // Scale smaller overtime.
    int     visible;
    float   dst; // Distance to player. Used for sorting to fix alpha blending.
};
*/


struct gbuffer_t {
    unsigned int normal_tex;
    unsigned int position_tex;
    unsigned int difspec_tex; // (Currently not used.)
    unsigned int depth_tex;

    unsigned int framebuffer;
    unsigned int depthbuffer;

    int res_x;
    int res_y;
};

// Sets the fog density automatically to 'render_dist'
// 'density' variable in fog_t struct will be ignored.
#define FOG_MODE_RENDERDIST 0

// Density can be controlled arbitrarily. range: 0 - 10.0
#define FOG_MODE_CUSTOM 1   // TODO



struct weather_t {
    Vector3 wind_dir;
    float   wind_strength;
    // ...
};


struct gamepad_t {
    int id;
    Vector2 Lstick;
    Vector2 Rstick;
    Vector2 Rstick_delta;
    float sensetivity;
};


// Game state "gst".
struct state_t {
    float time;
    float dt; // Previous frame time.
    Font font;
    platform_file_t cfgfile;
    struct config_t cfg;
    struct gamepad_t gamepad;

    unsigned int ubo[MAX_UBOS];
    unsigned int ssbo[MAX_SSBOS]; // <- (NOTE: CURRENTLY NOT USED)
    size_t num_prj_lights;

    
    struct player_t player;


    // ---- Weather Stuff ----

    struct fog_t      fog;
    struct light_t    sun;
    struct weather_t  weather;


    // ---- Shaders ----
    Shader               shaders[MAX_SHADERS];
    struct shaderutil_t  shader_u[MAX_SHADERS]; // Store uniform locations for shaders.


    // ---- Textures ----
    Texture       textures[MAX_TEXTURES];
    unsigned int  num_textures;


    // ---- Item Models ----
    Model  item_models[MAX_ITEM_MODELS];
    struct item_info_t item_info[MAX_ITEM_TYPES + MAX_WEAPON_MODELS];
    struct item_info_t* crosshair_item_info;
    float  item_info_screen_time;

    // ---- Weapon Models ----
    struct weapon_model_t weapon_models[MAX_WEAPON_MODELS];


    // ---- Lights ----
    // Light can be added to this array to be decayed/dimmed over time before disabling them completely.
    struct light_t decay_lights[MAX_DECAY_LIGHTS];
    size_t next_explosion_light_index;

    struct psystem_t psystems[MAX_PSYSTEMS];
    struct terrain_t terrain;

    struct npc_t npc;

    
    // ---- Enemies -----

    //struct spawn_system_t spawnsys;
    struct ent_spawnsys_t enemy_spawn_systems[MAX_ENEMY_TYPES];
    Model  enemy_models[MAX_ENEMY_MODELS];
    struct enemy_t enemies[MAX_ALL_ENEMIES];
    size_t num_enemies;
    size_t num_enemies_rendered;

    struct weapon_t enemy_weapons[MAX_ENEMY_WEAPONS];
    size_t num_enemy_weapons;


    // ---- Resolutions ----
    int res_x;
    int res_y;
    int ssao_res_x;
    int ssao_res_y;
    int shadow_res_x;
    int shadow_res_y;

    Vector2 screen_size; // "window size"


    // ---- Misc Models ----

    Model inventory_box_model;
    Model inventory_box_selected_model;

    // ---- For Rendering ----
    
    float render_dist; // Render distance.

    struct gbuffer_t gbuffer;  
    struct gbuffer_t shadow_gbuffers[MAX_SHADOW_LEVELS];

    Camera  shadow_cams[MAX_SHADOW_LEVELS];
    float   shadow_cam_height;
    float   shadow_bias;
 

    // Everything is rendered to this texture
    // and then post processed.
    RenderTexture2D env_render_target;
    RenderTexture2D env_render_downsample;
    RenderTexture2D inv_render_target; // Inventory is rendered here.

    // Bloom treshold is written here.
    // when post processing. bloom is aplied and mixed into 'env_render_target' texture
    RenderTexture2D bloomtresh_target;

    RenderTexture2D gui_render_target;

    int       ssao_enabled;
    Texture   ssao_noise_tex;
    Vector3*  ssao_kernel;
    int       show_only_ssao; // For debug.

    Matrix cam_view_matrix; // TODO: Move these
    Matrix cam_proj_matrix; //

    RenderTexture2D ssao_target;
    RenderTexture2D ssao_final;

    Color render_bg_color; // TODO: Remove this
    int running;
    int menu_open;
    int devmenu_open;
    
    float menu_slider_render_dist_v;
    Model skybox;

    RenderTexture2D bloom_downsamples[NUM_BLOOM_DOWNSAMPLES];



    // ---- Misc ----

    // This is for gui component.
    Texture  colorpick_tex;
    Image    colorpick_img;

    Material energy_liquid_material;

    int rseed; // Seed for randomgen functions.
    int debug;

    int xp_update_done;
    int xp_update_target;
    int xp_update_add;
    float xp_update_timer;
    float xp_value_f;

    int    has_audio;
    Sound  sounds[MAX_SOUNDS];

    float timebuf            [TIMEBUF_MAX_ELEMS][TIMEBUF_SIZE];
    size_t timebuf_indices   [TIMEBUF_MAX_ELEMS];

    uint64_t init_flags;  // What has been initialzied. Used by 'state_abort' function.
    int default_weapon_dropped;

    // For fine tuning weapon model offsets.
    Vector3 testmd_aim_offset;
    Vector3 testmd_rest_offset;
    Vector3 testmd_rest_rotation;
    Vector3 testmd_inspect_offset;
    Vector3 testmd_inspect_rotation;
    Vector3 testmd_energy_light_pos;

};

// NOTE: This function should only be used if errors happen while doing setup!
void state_abort(struct state_t* gst);

void state_timebuf_add(struct state_t* gst, int timebuf_elem, float time);
float state_average_timebuf(struct state_t* gst, int timebuf_elem);

void state_create_ubo(struct state_t* gst, int ubo_index, int binding_point, size_t size);
void state_create_ssbo(struct state_t* gst, int ssbo_index, int binding_point, size_t size);
void state_update_shader_uniforms(struct state_t* gst);
void state_update_frame(struct state_t* gst);
void state_update_shadow_cams(struct state_t* gst);
void state_update_shadow_map_uniforms(struct state_t* gst, int shader_index);

void add_item_namedesc(struct state_t* gst, int item_type, const char* name, const char* desc);

void create_explosion(struct state_t* gst, Vector3 position, float damage, float radius);
void set_render_dist(struct state_t* gst, float new_dist);

// 'shader_index' can be set to negative value so its not used.
void resample_texture(struct state_t* gst,
        RenderTexture2D to, RenderTexture2D from,
        int src_width, int src_height,
        int dst_width, int dst_height, int shader_index);
// Misc.
//void state_add_crithit_marker(struct state_t* gst, Vector3 position);



#endif
