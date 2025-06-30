#include <stdio.h>
#include <stdlib.h>

#include "weapon_model.h"
#include "state/state.h"

#include <rlgl.h>

#define CFGBUF_SIZE 256

#define CFG_ACCURACY_MIN 0
#define CFG_ACCURACY_MAX 100

#define CFG_FIRERATE_MIN 1
#define CFG_FIRERATE_MAX 30



void delete_weapon_model(struct state_t* gst, struct weapon_model_t* weapon_model) {
}



void load_weapon_model(
        struct state_t* gst,
        int weapon_model_index, // Index in 'gst->weapon_models' array.
        const char* config_file_path,
        const char* weapon_name,
        const char* weapon_desc
){
    struct weapon_model_t* weapon_model = &gst->weapon_models[weapon_model_index];
    weapon_model->incomplete = 1;

    printf("\033[94m|\033[0m %s: '%s'\n", __func__, config_file_path);

    platform_file_t cfgfile;
    platform_init_file(&cfgfile);

    if(!platform_read_file(&cfgfile, config_file_path)) {
        return;
    }
   

    char buf[CFGBUF_SIZE];

    // Setup 3D Model.

    if(!read_cfgvar(&cfgfile, "model_path", buf, CFGBUF_SIZE)) {
        return;
    }

    if(!FileExists(buf)) {
        fprintf(stderr, "\033[31m(ERROR) '%s': \"%s\" Does not exist\033[0m\n",
                __func__, buf);
        goto error;
    }
    
    printf("\033[94m|\033[0m  '%s'\n", buf);

    weapon_model->model = LoadModel(buf);
    weapon_model->model.materials[0] = LoadMaterialDefault();
    weapon_model->model.materials[0].maps[0].color = (Color){ 80, 80, 70, 255 };

    // This material is for "liquid magazine"
    weapon_model->model.materials[1] = LoadMaterialDefault();
    weapon_model->model.materials[1].shader = gst->shaders[ENERGY_LIQUID_SHADER];

    
    if(weapon_model->model.meshCount == 3) {
        weapon_model->has_redpoint_scope = 1;
        
        weapon_model->model.materials[2] = LoadMaterialDefault();
        weapon_model->model.materials[2].shader = gst->shaders[REDPOINT_SCOPE_SHADER];

    }
    else {
        weapon_model->has_redpoint_scope = 0;
    }
    //printf("%i\n", weapon_model->model.meshCount);


    // Setup stats.

    if(!read_cfgvar(&cfgfile, "damage", buf, CFGBUF_SIZE)) {
        return;
    }
    int weapon_damage = atoi(buf);
    
    
    if(!read_cfgvar(&cfgfile, "firerate", buf, CFGBUF_SIZE)) {
        return;
    }
    int firerate_i = atoi(buf);


    if(!read_cfgvar(&cfgfile, "color", buf, CFGBUF_SIZE)) {
        return;
    }
    int hex_color = atoi(buf);


    if(!read_cfgvar(&cfgfile, "projectile_speed", buf, CFGBUF_SIZE)) {
        return;
    }
    int projectile_speed = atoi(buf);


    if(!read_cfgvar(&cfgfile, "accuracy", buf, CFGBUF_SIZE)) {
        return;
    }
    int accuracy_i = atoi(buf);


    if(!read_cfgvar(&cfgfile, "critical_chance", buf, CFGBUF_SIZE)) {
        return;
    }
    int critical_chance = atoi(buf);


    if(!read_cfgvar(&cfgfile, "aim_offset_x", buf, CFGBUF_SIZE)) {
        return;
    }
    float aim_offset_x = atof(buf);
    
    if(!read_cfgvar(&cfgfile, "aim_offset_y", buf, CFGBUF_SIZE)) {
        return;
    }
    float aim_offset_y = atof(buf);
    
    if(!read_cfgvar(&cfgfile, "aim_offset_z", buf, CFGBUF_SIZE)) {
        return;
    }
    float aim_offset_z = atof(buf);

    if(!read_cfgvar(&cfgfile, "rest_offset_x", buf, CFGBUF_SIZE)) {
        return;
    }
    float rest_offset_x = atof(buf);

    if(!read_cfgvar(&cfgfile, "rest_offset_y", buf, CFGBUF_SIZE)) {
        return;
    }
    float rest_offset_y = atof(buf);

    if(!read_cfgvar(&cfgfile, "rest_offset_z", buf, CFGBUF_SIZE)) {
        return;
    }
    float rest_offset_z = atof(buf);


    if(!read_cfgvar(&cfgfile, "rest_rotation_x", buf, CFGBUF_SIZE)) {
        return;
    }
    float rest_rotation_x = atof(buf);

    if(!read_cfgvar(&cfgfile, "rest_rotation_y", buf, CFGBUF_SIZE)) {
        return;
    }
    float rest_rotation_y = atof(buf);

    if(!read_cfgvar(&cfgfile, "rest_rotation_z", buf, CFGBUF_SIZE)) {
        return;
    }
    float rest_rotation_z = atof(buf);

    if(!read_cfgvar(&cfgfile, "inspect_offset_x", buf, CFGBUF_SIZE)) {
        return;
    }
    float inspect_offset_x = atof(buf);

    if(!read_cfgvar(&cfgfile, "inspect_offset_y", buf, CFGBUF_SIZE)) {
        return;
    }
    float inspect_offset_y = atof(buf);

    if(!read_cfgvar(&cfgfile, "inspect_offset_z", buf, CFGBUF_SIZE)) {
        return;
    }
    float inspect_offset_z = atof(buf);

    if(!read_cfgvar(&cfgfile, "inspect_rotation_x", buf, CFGBUF_SIZE)) {
        return;
    }
    float inspect_rotation_x = atof(buf);

    if(!read_cfgvar(&cfgfile, "inspect_rotation_y", buf, CFGBUF_SIZE)) {
        return;
    }
    float inspect_rotation_y = atof(buf);

    if(!read_cfgvar(&cfgfile, "inspect_rotation_z", buf, CFGBUF_SIZE)) {
        return;
    }
    float inspect_rotation_z = atof(buf);

    if(!read_cfgvar(&cfgfile, "energy_light_offset_x", buf, CFGBUF_SIZE)) {
        return;
    }
    float energy_light_offset_x = atof(buf);

    if(!read_cfgvar(&cfgfile, "energy_light_offset_y", buf, CFGBUF_SIZE)) {
        return;
    }
    float energy_light_offset_y = atof(buf);

    if(!read_cfgvar(&cfgfile, "energy_light_offset_z", buf, CFGBUF_SIZE)) {
        return;
    }
    float energy_light_offset_z = atof(buf);

    if(!read_cfgvar(&cfgfile, "prjfx_offset", buf, CFGBUF_SIZE)) {
        return;
    }
    weapon_model->prjfx_offset = atof(buf);


    if(!read_cfgvar(&cfgfile, "recoil_anim_kickback", buf, CFGBUF_SIZE)) {
        return;
    }
    weapon_model->recoil_anim_kickback = atof(buf);

    if(!read_cfgvar(&cfgfile, "recoil", buf, CFGBUF_SIZE)) {
        return;
    }
    weapon_model->recoil = atof(buf);

    if(!read_cfgvar(&cfgfile, "knockback", buf, CFGBUF_SIZE)) {
        return;
    }
    weapon_model->stats.knockback = atof(buf);

    if(!read_cfgvar(&cfgfile, "lqmag_capacity", buf, CFGBUF_SIZE)) {
        return;
    }
    float lqmag_capacity = atof(buf);

    if(!read_cfgvar(&cfgfile, "has_scope", buf, CFGBUF_SIZE)) {
        return;
    }
    weapon_model->has_scope = atoi(buf);

    if(!read_cfgvar(&cfgfile, "rarity", buf, CFGBUF_SIZE)) {
        return;
    }
    weapon_model->rarity = atoi(buf);



    float accuracy_f = map(
            (float)accuracy_i,
            CFG_ACCURACY_MIN,
            CFG_ACCURACY_MAX,
            WEAPON_ACCURACY_MIN,
            WEAPON_ACCURACY_MAX
            );

    float firerate_f = map(
            (float)firerate_i,
            CFG_FIRERATE_MIN,
            CFG_FIRERATE_MAX,
            0.07,
            1.0
            );
    
    weapon_model->item_info_index = MAX_ITEM_TYPES + weapon_model_index;
    weapon_model->aim_offset = (Vector3){ aim_offset_x, aim_offset_y, aim_offset_z };
    weapon_model->rest_offset = (Vector3){ rest_offset_x, rest_offset_y, rest_offset_z };
    weapon_model->rest_rotation = (Vector3){ rest_rotation_x, rest_rotation_y, rest_rotation_z };
    weapon_model->inspect_offset = (Vector3){ inspect_offset_x, inspect_offset_y, inspect_offset_z };
    weapon_model->inspect_rotation = (Vector3){ inspect_rotation_x, inspect_rotation_y, inspect_rotation_z };
    weapon_model->energy_light_offset = (Vector3){ energy_light_offset_x, energy_light_offset_y, energy_light_offset_z };

    weapon_model->recoil_anim_done = 0;
    weapon_model->recoil_anim_timer = 0.0;
    weapon_model->recoil_anim_value = 0.0;
    weapon_model->firerate_timer = 0.0;
    weapon_model->firerate = firerate_f;

    int red = (hex_color >> 16) & 0xFF;
    int grn = (hex_color >> 8) & 0xFF;
    int blu = (hex_color) & 0xFF;

    weapon_model->stats = (struct weapon_t) {
        .gid = PLAYER_WEAPON_GID,
        .accuracy = accuracy_f,
        .damage = weapon_damage,
        .critical_chance = critical_chance,
        .critical_mult = 5.0,
        .prj_speed = projectile_speed,
        .prj_max_lifetime = 10.0,
        .prj_hitbox_size = (Vector3) { 1.0, 1.0, 1.0 },
        .prj_scale = 1.0,
        .color = (Color){ red, grn, blu, 200 },
        .overheat_temp = -1
    };

    init_weapon_lqmag(gst, &weapon_model->stats, lqmag_capacity);
    add_item_namedesc(gst, weapon_model->item_info_index, weapon_name, weapon_desc);

    weapon_model->gravity_offset   = (Vector3){ 0, 0, 0 };
    weapon_model->gravity_velocity = (Vector3){ 0, 0, 0 };
    weapon_model->render_offset    = (Vector3){ 0, 0, 0 };
    weapon_model->gravity_friction = WMODEL_LOW_GRAVITY_FRICTION;

error:
    platform_close_file(&cfgfile);
}

void render_weapon_model(struct state_t* gst, struct weapon_model_t* weapon_model, Matrix transform) {


    Vector3 light_pos = weapon_model->energy_light_offset;
    light_pos = Vector3Transform(light_pos, transform);


    // Render liquid magazine level.
    shader_setu_float(gst, ENERGY_LIQUID_SHADER, U_ENERGY_CONTAINER_LEVEL,
            &weapon_model->stats.lqmag.ammo_level);
    
    shader_setu_float(gst, ENERGY_LIQUID_SHADER, U_ENERGY_CONTAINER_CAPACITY,
            &weapon_model->stats.lqmag.capacity);
    
    shader_setu_color(gst, ENERGY_LIQUID_SHADER, U_ENERGY_COLOR,
            &weapon_model->stats.color);

    DrawMesh(
            weapon_model->model.meshes[1],
            weapon_model->model.materials[1],
            transform
            );

    if(weapon_model->has_redpoint_scope) {
        rlDisableDepthMask();
        rlDisableBackfaceCulling();
        DrawMesh(
               weapon_model->model.meshes[2],
               weapon_model->model.materials[2],
               transform
               );
        rlEnableDepthMask();
        rlEnableBackfaceCulling();
    }

    // Rest of the model.
    DrawMesh(
            weapon_model->model.meshes[0],
            weapon_model->model.materials[0],
            transform
            );
}

void use_weapon_model_test_offsets(struct state_t* gst, struct weapon_model_t* weapon_model) {

    weapon_model->aim_offset = gst->testmd_aim_offset;
    weapon_model->rest_offset = gst->testmd_rest_offset;
    weapon_model->rest_rotation = gst->testmd_rest_rotation;
    weapon_model->inspect_offset = gst->testmd_inspect_offset;
    weapon_model->inspect_rotation = gst->testmd_inspect_rotation;
    weapon_model->energy_light_offset = gst->testmd_energy_light_pos;
    weapon_model->prjfx_offset = gst->testmd_prjfx_offset;
    weapon_model->stats.prj_speed = gst->testmd_prj_speed;

    float firerate_f = map(
            (float)gst->testmd_firerate,
            CFG_FIRERATE_MIN,
            CFG_FIRERATE_MAX,
            0.07,
            1.0
            );

    float accuracy_f = map(
            (float)gst->testmd_accuracy,
            CFG_ACCURACY_MIN,
            CFG_ACCURACY_MAX,
            WEAPON_ACCURACY_MIN,
            WEAPON_ACCURACY_MAX
            );

    weapon_model->firerate = firerate_f;
    weapon_model->stats.accuracy = accuracy_f;
 
}





