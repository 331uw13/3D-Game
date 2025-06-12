#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "state.h"
#include "state/state_free.h"
#include "input.h"
#include "util.h"
#include "memory.h"

#include <raymath.h>
#include <rlgl.h>

#include "../particle_systems/explosion_psys.h"
#include "../particle_systems/weapon_psys.h"
/*
void state_setup_gbuffer(struct state_t* gst) {
    gst->gbuffer = (struct gbuffer_t) { 0 };

    gst->gbuffer.framebuffer = rlLoadFramebuffer();
    if(!gst->gbuffer.framebuffer) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Failed to create framebuffer\033[0m\n",
                __func__);
        return;
    }

    float scale;

    switch(gst->cfg.ssao_quality) {
        default:
        case CFG_SSAO_QLOW:
            scale = 2.0;
            break;
        
        case CFG_SSAO_QMED:
            scale = 1.35;
            break;
        
        case CFG_SSAO_QHIGH:
            scale = 1.0;
            break;
    }

    gst->gbuffer.res_x = gst->res_x / scale;
    gst->gbuffer.res_y = gst->res_y / scale;

    printf("Creating gbuffer: %ix%i\n", gst->gbuffer.res_x, gst->gbuffer.res_y);
    rlEnableFramebuffer(gst->gbuffer.framebuffer);
    

    // Use 32 bits per channel to avoid floating point precision loss.
    
    // Positions.
    gst->gbuffer.position_tex = rlLoadTexture(NULL, gst->gbuffer.res_x, gst->gbuffer.res_y, 
                RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32, 1);
    
    // Normals.
    gst->gbuffer.normal_tex = rlLoadTexture(NULL, gst->gbuffer.res_x, gst->gbuffer.res_y, 
                RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32, 1);
    
    // Diffuse specular colors. (This is not currently used.)
    gst->gbuffer.difspec_tex = rlLoadTexture(NULL, gst->gbuffer.res_x, gst->gbuffer.res_y, 
                RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8, 1);

    // Depth.
    gst->gbuffer.depth_tex = rlLoadTexture(NULL, gst->gbuffer.res_x, gst->gbuffer.res_y, 
                RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32, 1);


    gst->gbuffer.depthbuffer = rlLoadTextureDepth(gst->gbuffer.res_x, gst->gbuffer.res_y, 1);


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

void state_setup_shadow_gbuffer(struct state_t* gst) {
    gst->shadow_gbuffer = (struct gbuffer_t) { 0 };

    gst->shadow_gbuffer.framebuffer = rlLoadFramebuffer();
    if(!gst->shadow_gbuffer.framebuffer) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Failed to create framebuffer\033[0m\n",
                __func__);
        return;
    }

    float scale = 1.0;

    gst->shadow_gbuffer.res_x = gst->res_x / scale;
    gst->shadow_gbuffer.res_y = gst->res_y / scale;

    printf("Creating shadow gbuffer: %ix%i\n", gst->shadow_gbuffer.res_x, gst->shadow_gbuffer.res_y);
    rlEnableFramebuffer(gst->shadow_gbuffer.framebuffer);
    

    // Use 32 bits per channel to avoid floating point precision loss.
    
    // Positions.
    gst->shadow_gbuffer.position_tex = rlLoadTexture(NULL, gst->shadow_gbuffer.res_x, gst->shadow_gbuffer.res_y, 
                RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32, 1);
    
    // Normals.
    gst->shadow_gbuffer.normal_tex = rlLoadTexture(NULL, gst->shadow_gbuffer.res_x, gst->shadow_gbuffer.res_y, 
                RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32, 1);
    
    // Diffuse specular colors. (This is not currently used.)
    gst->shadow_gbuffer.difspec_tex = rlLoadTexture(NULL, gst->shadow_gbuffer.res_x, gst->shadow_gbuffer.res_y, 
                RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8, 1);

    // Depth.
    gst->shadow_gbuffer.depth_tex = rlLoadTexture(NULL, gst->shadow_gbuffer.res_x, gst->shadow_gbuffer.res_y, 
                RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32, 1);


    gst->shadow_gbuffer.depthbuffer = rlLoadTextureDepth(gst->shadow_gbuffer.res_x, gst->shadow_gbuffer.res_y, 1);


    rlActiveDrawBuffers(4);

    // Attach textures to the framebuffer.
    rlFramebufferAttach(gst->shadow_gbuffer.framebuffer, gst->shadow_gbuffer.position_tex,
            RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
    
    rlFramebufferAttach(gst->shadow_gbuffer.framebuffer, gst->shadow_gbuffer.normal_tex,
            RL_ATTACHMENT_COLOR_CHANNEL1, RL_ATTACHMENT_TEXTURE2D, 0);

    rlFramebufferAttach(gst->shadow_gbuffer.framebuffer, gst->shadow_gbuffer.difspec_tex,
            RL_ATTACHMENT_COLOR_CHANNEL2, RL_ATTACHMENT_TEXTURE2D, 0);

    rlFramebufferAttach(gst->shadow_gbuffer.framebuffer, gst->shadow_gbuffer.depth_tex,
            RL_ATTACHMENT_COLOR_CHANNEL3, RL_ATTACHMENT_TEXTURE2D, 0);

    // Attach depth buffer.
    rlFramebufferAttach(gst->shadow_gbuffer.framebuffer, gst->shadow_gbuffer.depthbuffer,
            RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);


    if(!rlFramebufferComplete(gst->shadow_gbuffer.framebuffer)) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Framebuffer is not complete!\033[0m\n",
                __func__);
    }    
}

*/

void state_abort(struct state_t* gst, const char* reason, const char* from_func, const char* from_file) {
    fprintf(stderr, 
            "(FATAL) %s called from '%s()' @ \"%s\"\n"
            "Abort reason: %s\n"
            "\n"
            ,
            __func__, from_func, from_file, reason);

    state_free_everything(gst);

    CloseWindow();
    exit(EXIT_FAILURE);
}

void state_timebuf_add(struct state_t* gst, int timebuf_elem, float time) {
    
    if(timebuf_elem > TIMEBUF_MAX_ELEMS) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Invalid timebuf_elem\033[0m\n",
                __func__);
        return;
    }

    if(time < -0.000005) {
        printf("\033[35m(WARNING) '%s': Adding time (%f) into %i, is negative time.\033[0m\n",
                __func__, time, timebuf_elem);
    }


    size_t* index = &gst->timebuf_indices[timebuf_elem];
    gst->timebuf[timebuf_elem][*index] = time;

    *index += 1;
    if(*index >= TIMEBUF_SIZE) {
        *index = 0;
    }
}

float state_average_timebuf(struct state_t* gst, int timebuf_elem) {
    float average_time = -1.0;

    if(timebuf_elem >= TIMEBUF_MAX_ELEMS) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Invalid timebuf_elem\033[0m\n",
                __func__);
        goto error;
    }

    average_time = 0.0;

    for(size_t i = 0; i < TIMEBUF_SIZE; i++) {
        average_time += gst->timebuf[timebuf_elem][i];
    }

    average_time /= ((float)TIMEBUF_SIZE);

error:
    return average_time;
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

void state_create_ssbo(struct state_t* gst, int ssbo_index, int binding_point, size_t size) {
    gst->ssbo[ssbo_index] = 0;
    
    glGenBuffers(1, &gst->ssbo[ssbo_index]);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gst->ssbo[ssbo_index]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, NULL, GL_DYNAMIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point, gst->ssbo[ssbo_index]);
    //glBindBufferRange(GL_SHADER_STORAGE_BUFFER, binding_point, gst->ssbo[ssbo_index], 0, size);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}


void state_update_shadow_map_uniforms(struct state_t* gst, int shader_index) {

    // Update shadow maps.

    Vector2 shadow_res = (Vector2){ gst->shadow_res_x, gst->shadow_res_y };
    float aspect_ratio = shadow_res.x / shadow_res.y;
    
    shader_setu_vec2(gst, shader_index, U_SHADOW_RES, &shadow_res);
    shader_setu_float(gst, shader_index, U_SHADOW_BIAS, &gst->shadow_bias);

    int shadow_map_slot = 10;
    for(int i = 0; i < MAX_SHADOW_LEVELS; i++) {
        Camera* shadow_cam = &gst->shadow_cams[i];

        //shader_setu_matrix(gst, DEFAULT_SHADER, U_SHADOWVIEW_MATRIX, GetCameraViewMatrix(shadow_cam));
        //shader_setu_matrix(gst, DEFAULT_SHADER, U_SHADOWPROJ_MATRIX, GetCameraProjectionMatrix(shadow_cam, aspect));

        int view_matrix_loc = GetShaderLocation(gst->shaders[shader_index], TextFormat("u_shadowview_matrix[%i]",i));
        int proj_matrix_loc = GetShaderLocation(gst->shaders[shader_index], TextFormat("u_shadowproj_matrix[%i]",i));
        int fovy_loc        = GetShaderLocation(gst->shaders[shader_index], TextFormat("u_shadow_fov[%i]",i));
        
        Matrix cam_view_matrix = GetCameraViewMatrix(shadow_cam);
        Matrix cam_proj_matrix = GetCameraProjectionMatrix(shadow_cam, aspect_ratio);

        SetShaderValueMatrix(gst->shaders[shader_index], view_matrix_loc, cam_view_matrix);
        SetShaderValueMatrix(gst->shaders[shader_index], proj_matrix_loc, cam_proj_matrix);

        SetShaderValueV(gst->shaders[shader_index], fovy_loc, &shadow_cam->fovy, SHADER_UNIFORM_FLOAT, 1);

        int shadow_map_loc = GetShaderLocation(gst->shaders[shader_index], 
                TextFormat("shadow_maps[%i]", i));


        rlEnableShader(gst->shaders[shader_index].id);
        rlActiveTextureSlot(shadow_map_slot);
        rlEnableTexture(gst->shadow_gbuffers[i].position_tex);
        rlSetUniform(shadow_map_loc, &shadow_map_slot, SHADER_UNIFORM_INT, 1);

        shadow_map_slot++;
    }
}

static void state_update_biome_texture_uniforms(struct state_t* gst) {
    
    int tex_slot = 4;

    for(int i = 0; i < MAX_BIOME_TYPES; i++) {
        rlEnableShader(gst->shaders[DEFAULT_SHADER].id);
        rlActiveTextureSlot(tex_slot);
        rlEnableTexture(gst->terrain.biome_materials[i].maps[MATERIAL_MAP_DIFFUSE].texture.id);
        int biome_tex_loc = GetShaderLocation(gst->shaders[DEFAULT_SHADER],
                TextFormat("biome_groundtex[%i]", i));
        rlSetUniform(biome_tex_loc, &tex_slot, SHADER_UNIFORM_INT, 1);
        tex_slot++;
    }
}


// TODO: Optimize shader uniforms. (not everything has to be updated all the time.)

void state_update_shader_uniforms(struct state_t* gst) {

    // Update Player view position.
    shader_setu_vec3(gst, DEFAULT_SHADER,      U_CAMPOS, &gst->player.cam.position);
    shader_setu_vec3(gst, WATER_SHADER,        U_CAMPOS, &gst->player.cam.position);
    shader_setu_vec3(gst, FOLIAGE_SHADER,      U_CAMPOS, &gst->player.cam.position);
    shader_setu_vec3(gst, FOLIAGE_WIND_SHADER, U_CAMPOS, &gst->player.cam.position);
    shader_setu_vec3(gst, FOG_PARTICLE_SHADER, U_CAMPOS, &gst->player.cam.position);
    shader_setu_vec3(gst, CLOUD_PARTICLE_SHADER, U_CAMPOS, &gst->player.cam.position);
    shader_setu_vec3(gst, SKY_SHADER,            U_CAMPOS, &gst->player.cam.position);
    shader_setu_vec3(gst, FRACTAL_MODEL_SHADER,  U_CAMPOS, &gst->player.cam.position);
    shader_setu_vec3(gst, FRACTAL_BERRY_SHADER,  U_CAMPOS, &gst->player.cam.position);

    // Update screen size.
    Vector2 resolution = (Vector2) {
        gst->res_x, gst->res_y
    };
    
    shader_setu_vec2(gst, POSTPROCESS_SHADER,      U_SCREEN_SIZE, &resolution);
    shader_setu_vec2(gst, SSAO_SHADER,             U_SCREEN_SIZE, &resolution);
    shader_setu_vec2(gst, SCOPE_CROSSHAIR_SHADER,  U_SCREEN_SIZE, &gst->screen_size);


    // Update time
    shader_setu_float(gst, DEFAULT_SHADER,         U_TIME, &gst->time);
    shader_setu_float(gst, FOLIAGE_SHADER,         U_TIME, &gst->time);
    shader_setu_float(gst, FOLIAGE_WIND_SHADER,    U_TIME, &gst->time);
    shader_setu_float(gst, GBUFFER_FOLIAGE_WIND_SHADER, U_TIME, &gst->time);
    shader_setu_float(gst, POSTPROCESS_SHADER,     U_TIME, &gst->time);
    shader_setu_float(gst, CLOUD_PARTICLE_SHADER,  U_TIME, &gst->time);
    shader_setu_float(gst, SKY_SHADER,             U_TIME, &gst->time);
    shader_setu_float(gst, ENERGY_LIQUID_SHADER,   U_TIME, &gst->time);
    shader_setu_float(gst, FRACTAL_MODEL_SHADER,   U_TIME, &gst->time);
    shader_setu_float(gst, FRACTAL_MODEL_GBUFFER_SHADER, U_TIME, &gst->time);



    // Update misc.

    shader_setu_float(gst, SSAO_SHADER, U_RENDER_DIST, &gst->render_dist);
    shader_setu_int(gst, POSTPROCESS_SHADER, U_SSAO_ENABLED, &gst->ssao_enabled);
    shader_setu_int(gst, POSTPROCESS_SHADER, U_ANYGUI_OPEN, &gst->player.any_gui_open);
    shader_setu_int(gst, SSAO_SHADER, U_SSAO_KERNEL_SAMPLES, &gst->cfg.ssao_kernel_samples);  

    // Weather data.
    // TODO: Uniform buffer for weather data.
    shader_setu_float(gst, FOLIAGE_WIND_SHADER, U_WIND_STRENGTH, &gst->weather.wind_strength);
    shader_setu_vec3(gst,  FOLIAGE_WIND_SHADER, U_WIND_DIR, &gst->weather.wind_dir);
    shader_setu_float(gst, GBUFFER_FOLIAGE_WIND_SHADER, U_WIND_STRENGTH, &gst->weather.wind_strength);
    shader_setu_vec3(gst,  GBUFFER_FOLIAGE_WIND_SHADER, U_WIND_DIR, &gst->weather.wind_dir);
    
    // -------

    //shader_setu_color(gst, ENERGY_LIQUID_SHADER, U_ENERGY_COLOR, &gst->player.weapon.color);

    shader_setu_float(gst, SKY_SHADER, U_RENDER_DIST, &gst->render_dist);
    shader_setu_color(gst, SKY_SHADER, U_SUN_COLOR, &gst->sun.color);

    state_update_shadow_map_uniforms(gst, DEFAULT_SHADER);
    state_update_shadow_map_uniforms(gst, FOLIAGE_SHADER);
    state_update_shadow_map_uniforms(gst, FRACTAL_MODEL_SHADER);
    state_update_biome_texture_uniforms(gst);

    shader_setu_float(gst, DEFAULT_SHADER, U_TERRAIN_LOWEST, &gst->terrain.lowest_point);
    shader_setu_float(gst, DEFAULT_SHADER, U_TERRAIN_HIGHEST, &gst->terrain.highest_point);

    for(int i = 0; i < MAX_BIOME_TYPES; i++) {
        int biome_ylevel_loc = GetShaderLocation(gst->shaders[DEFAULT_SHADER],
                TextFormat("biome_ylevels[%i]", i));
        SetShaderValueV(gst->shaders[DEFAULT_SHADER], biome_ylevel_loc, &gst->terrain.biome_ylevels[i],
                SHADER_UNIFORM_VEC2, 1);
    }
    /*
    struct chunk_t* last_chunk = &gst->terrain.chunks[gst->terrain.num_chunks-1];
    Vector3 sizeoff = (Vector3){ gst->terrain.chunk_size*gst->terrain.scaling, 0, gst->terrain.chunk_size*gst->terrain.scaling };
    printf("%f\n", Vector3Length(Vector3Add(last_chunk->center_pos, sizeoff)));
    */
}

void state_update_shadow_cams(struct state_t* gst) {
    for(int i = 0; i < MAX_SHADOW_LEVELS; i++) {
        Camera* cam = &gst->shadow_cams[i];
        cam->target = (Vector3){0, 0, 0};
        cam->target.x =     (gst->player.cam.position.x - cam->target.x);
        cam->target.z = 1.0+(gst->player.cam.position.z - cam->target.z);
        //cam->target.y =     (gst->player.cam.position.y - cam->target.y);
        cam->position = gst->player.cam.position;
        cam->position.y += gst->shadow_cam_height;
    }
}

static void state_update_lights(struct state_t* gst) {

    // Map light ssbo.
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gst->ssbo[CHUNK_LIGHTS_SSBO]);
    void* light_data = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);

    if(!light_data) {
        fprintf(stderr, "'%s' OpenGL Error: 0x%x\n", __func__, glGetError());
        return;
    }


    float buffer[12 * MAX_LIGHTS] = { 0 };
    gst->num_lights_mvram = 0;
    
    int counter = 0;

    for(uint16_t i = 0; i < MAX_LIGHTS; i++) {
        struct light_t* light = &gst->lights[i];
        if(!light->enabled) {
            continue;
        }

        // Dont send lights that are not visible.

        // Color.
        buffer[counter+0] = (float)light->color.r / 255.0;
        buffer[counter+1] = (float)light->color.g / 255.0;
        buffer[counter+2] = (float)light->color.b / 255.0;
        buffer[counter+3] = 1.0;

        buffer[counter+4] = light->position.x;
        buffer[counter+5] = light->position.y;
        buffer[counter+6] = light->position.z;
        buffer[counter+7] = 0;
        
        buffer[counter+8] = light->strength;
        buffer[counter+9] = light->radius;
        buffer[counter+10] = 0;
        buffer[counter+11] = 0;

        counter += 12;
        gst->num_lights_mvram++;
    }

    memmove(
            light_data,
            buffer,
            sizeof(buffer)
            );

    // Set number of lights for the frame.
    memmove(
            light_data + sizeof(buffer),
            &gst->num_lights_mvram,
            sizeof(int)
            );

    // Unmap light SSBO.
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}


// NOTE: Do not render from here
void state_update_frame(struct state_t* gst) {
    
    gst->player.any_gui_open = 
        gst->menu_open || gst->player.inventory.open;


    handle_userinput(gst);
    
    if(gst->menu_open) {
        return;
    }

    gst->crosshair_item_info = NULL;


    // For testing new weapon model configs.
    /*
    if(gst->player.item_in_hands) {
        if(gst->player.item_in_hands->is_weapon_item) {
            use_weapon_model_test_offsets(gst, &gst->player.item_in_hands->weapon_model);
        }
    }
    */


    // For testing purposes drop all weapon types.
    if((gst->time - gst->loading_time) > 1.0 && !gst->default_weapon_dropped) {

        float drop_xoff =  0.0;
        float drop_zoff =  10.0;

        for(int type = 0; type < MAX_WEAPON_MODELS; type++) {

            Vector3 pos = (Vector3) {
                gst->player.position.x + drop_xoff,
                gst->player.position.y + 8.0,
                gst->player.position.z + drop_zoff
            };

            drop_xoff += 20;

            struct item_t weapon_item = get_weapon_model_item(gst, type);
            drop_item(gst, FIND_ITEM_CHUNK, pos, &weapon_item);
        }

        
        gst->default_weapon_dropped = 1;
    }

    // Update Enemies.
    for(size_t i = 0; i < gst->num_enemies; i++) {
        update_enemy(gst, &gst->enemies[i]);
    }

    //update_enemy_spawn_systems(gst); 


    // Update Items.
    for(int i = 0; i < gst->terrain.num_rendered_chunks; i++) {
        chunk_update_items(gst, gst->terrain.rendered_chunks[i]);
    }
    

    // Particle Systems.
    // (updated only if needed)
    update_psystem(gst, &gst->psystems[PLAYER_WEAPON_PSYS]);
    update_psystem(gst, &gst->psystems[ENEMY_WEAPON_PSYS]);
    update_psystem(gst, &gst->psystems[PROJECTILE_ENVHIT_PSYS]);
    update_psystem(gst, &gst->psystems[ENEMY_HIT_PSYS]);
    update_psystem(gst, &gst->psystems[FOG_EFFECT_PSYS]);
    update_psystem(gst, &gst->psystems[PLAYER_HIT_PSYS]);
    update_psystem(gst, &gst->psystems[EXPLOSION_PSYS]);
    update_psystem(gst, &gst->psystems[ENEMY_GUNFX_PSYS]);
    update_psystem(gst, &gst->psystems[CLOUD_PSYS]);
    update_psystem(gst, &gst->psystems[PRJ_TRAIL_PSYS]);
    

    // Update Misc.
    update_biome_envblend(gst);
    update_npc(gst, &gst->npc);
    player_update(gst, &gst->player);
    state_update_shadow_cams(gst);


    // XP.
    if(!gst->xp_update_done) {    
        gst->xp_update_timer += gst->dt;
        if(gst->xp_update_timer >= 0.01) {
            gst->xp_update_timer = 0;

            if(gst->xp_update_target < gst->player.xp) {
                gst->player.xp -= gst->xp_update_add;
                if(gst->player.xp <= gst->xp_update_target) {
                    gst->player.xp = gst->xp_update_target;
                    gst->xp_update_done = 1;
                }
            }
            else {
                gst->player.xp += gst->xp_update_add;
                if(gst->player.xp >= gst->xp_update_target) {
                    gst->player.xp = gst->xp_update_target;
                    gst->xp_update_done = 1;
                }
            }
        }
    }

    gst->player.wants_to_pickup_item = 0;

    // Move enabled lights to lights ssbo before rendering anything.
    state_update_lights(gst);
    
    /*
    if(gst->new_render_dist_scheduled) {
        gst->new_render_dist_scheduled = 0;
        set_render_dist(gst, gst->new_render_dist);
    }
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
    //set_light(gst, &exp_light, LIGHTS_UBO);
    //add_decay_light(gst, &exp_light, 8.0);

    /*
    gst->next_explosion_light_index++;
    if(gst->next_explosion_light_index >= MAX_EXPLOSION_LIGHTS) {
        gst->next_explosion_light_index = MAX_STATIC_LIGHTS;
    }
    */

    if(gst->has_audio) {
        SetSoundVolume(gst->sounds[ENEMY_EXPLOSION_SOUND], get_volume_dist(gst->player.position, position));
        SetSoundPitch(gst->sounds[ENEMY_EXPLOSION_SOUND], 1.0 - RSEEDRANDOMF(0.0, 0.3));
        PlaySound(gst->sounds[ENEMY_EXPLOSION_SOUND]);
    }

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

    gst->old_render_dist = gst->render_dist;
    gst->render_dist = new_dist;

    // Small number for near plane avoids 
    rlSetClipPlanes(0.160000, gst->render_dist+3000.0);

    gst->menu_slider_render_dist_v = gst->render_dist;

    // Figure out how many chunks may be rendered at once.
    // This is bad because it is not accurate at all... but will do for now.
    // Also doesnt take in count the "frustrum culling" for chunks.

    gst->terrain.num_max_visible_chunks = 8; // take in count error.
    for(size_t i = 0; i < gst->terrain.num_chunks; i++) {
        struct chunk_t* chunk = &gst->terrain.chunks[i];

        float dst = Vector3Length((Vector3){ chunk->center_pos.x, 0, chunk->center_pos.z });

        if(dst <= gst->render_dist) {
            gst->terrain.num_max_visible_chunks++;
        }
    }

    printf("'%s': New render distance: %0.3f\n", __func__, new_dist);
    printf("'%s': Predicted visible chunks: %i (without frustrum culling)\n",
            __func__, gst->terrain.num_max_visible_chunks);

    // Allocate/Reallocate space for foliage.
    // When rendering terrain: foliage are copied into bigger arrays and then rendered all at once(per type)
    // ^ This will reduce number of draw calls.

    for(size_t i = 0; i < MAX_FOLIAGE_TYPES; i++) {
        struct foliage_rdata_t* f_rdata = &gst->terrain.foliage_rdata[i];

        if(f_rdata->matrices) {
            free(f_rdata->matrices);
            f_rdata->matrices = NULL;
        }

        if(gst->terrain.foliage_max_perchunk[i] == 0) {
            fprintf(stderr, "\033[31m(ERROR) '%s': foliage_max_perchunk must not be zero!\n"
                            " It has been set to 1 to avoid crashing.\033[0m\n",
                            __func__);
            gst->terrain.foliage_max_perchunk[i] = 1;
        }

        f_rdata->matrices_size 
            = gst->terrain.num_max_visible_chunks
            * gst->terrain.foliage_max_perchunk[i];

        f_rdata->matrices = malloc(f_rdata->matrices_size * sizeof *f_rdata->matrices);
   
        printf("\033[35m(%p)\033[0m\n", f_rdata->matrices);
    }


    // Resize the 'rendered_chunks' array.

    const size_t rchunks_new_size 
        = gst->terrain.num_max_visible_chunks
        * sizeof *gst->terrain.rendered_chunks;

    if(gst->terrain.rendered_chunks) {
        struct chunk_t** rchunks_tmpptr 
            = realloc(gst->terrain.rendered_chunks, rchunks_new_size);
        if(!rchunks_tmpptr) {
            STATE_ABORT(gst, "Memory error!");
        }
        gst->terrain.rendered_chunks = rchunks_tmpptr;

    }
    else {
        gst->terrain.rendered_chunks = malloc(rchunks_new_size);
    }


    if(gst->fog.mode == FOG_MODE_RENDERDIST) {
        set_fog_settings(gst, &gst->fog);
    }
}

void resample_texture(struct state_t* gst, 
        RenderTexture2D to, RenderTexture2D from,
        int src_width, int src_height, int dst_width,
        int dst_height, int shader_index
){

    BeginTextureMode(to);
    ClearBackground((Color){0, 0, 0, 255});
    if(shader_index >= 0) {
        BeginShaderMode(gst->shaders[shader_index]);
    }

    DrawTexturePro(
            from.texture,
            (Rectangle){0, 0, src_width, -src_height },
            (Rectangle){0, 0, dst_width, -dst_height },
            (Vector2){0}, 0.0, WHITE
            );

    if(shader_index >= 0) {
        EndShaderMode();
    }
    EndTextureMode();
}

void add_item_namedesc(struct state_t* gst, int item_type, const char* name, const char* desc) {

    if(!name) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Name must be set.\033[0m\n",
                __func__);
        return;
    }

    struct item_info_t* itemi = &gst->item_info[item_type];

    itemi->desc_size = 0;
    itemi->name_size = 0;

    itemi->name_size = strlen(name);
    memmove(itemi->name, name, itemi->name_size);

    if(desc) {
        itemi->desc_size = strlen(desc);
        memmove(itemi->desc, desc, itemi->desc_size);
    }

}

void schedule_new_render_dist(struct state_t* gst, float new_dist) {
    gst->new_render_dist_scheduled = 1;
    gst->new_render_dist = new_dist;
}

