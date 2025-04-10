#include <stdio.h>
#include <stdlib.h>

#include "state.h"
#include "input.h"
#include "util.h"

#include <raymath.h>
#include <rlgl.h>

#include "../particle_systems/explosion_psys.h"
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

void state_create_ubo(struct state_t* gst, int ubo_index, int binding_point, size_t size) {
    gst->ubo[ubo_index] = 0;

    glGenBuffers(1, &gst->ubo[ubo_index]);
    glBindBuffer(GL_UNIFORM_BUFFER, gst->ubo[ubo_index]);
    glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, gst->ubo[ubo_index]);
    glBindBufferRange(GL_UNIFORM_BUFFER, binding_point, gst->ubo[ubo_index], 0, size);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
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

void state_update_shader_uniforms(struct state_t* gst) {

    // Update Player view position.
    shader_setu_vec3(gst, DEFAULT_SHADER,      U_CAMPOS, &gst->player.cam.position);
    shader_setu_vec3(gst, WATER_SHADER,        U_CAMPOS, &gst->player.cam.position);
    shader_setu_vec3(gst, FOLIAGE_SHADER,      U_CAMPOS, &gst->player.cam.position);
    shader_setu_vec3(gst, FOG_PARTICLE_SHADER, U_CAMPOS, &gst->player.cam.position);
    shader_setu_vec3(gst, SKY_SHADER,          U_CAMPOS, &gst->player.cam.position);


    // Update screen size.

    Vector2 resolution = (Vector2) {
        gst->res_x, gst->res_y
    };
    
    shader_setu_vec2(gst, POSTPROCESS_SHADER,      U_SCREEN_SIZE, &resolution);
    shader_setu_vec2(gst, SSAO_SHADER,             U_SCREEN_SIZE, &resolution);
    

    // Update time
    shader_setu_float(gst, DEFAULT_SHADER,         U_TIME, &gst->time);
    shader_setu_float(gst, FOLIAGE_SHADER,         U_TIME, &gst->time);
    shader_setu_float(gst, POSTPROCESS_SHADER,     U_TIME, &gst->time);
    shader_setu_float(gst, WATER_SHADER,           U_TIME, &gst->time);

    // Update water level
    shader_setu_float(gst, DEFAULT_SHADER, U_WATERLEVEL, &gst->terrain.water_ylevel);



    // Update misc.
   
    shader_setu_float(gst, SSAO_SHADER, U_RENDER_DIST, &gst->render_dist);
    shader_setu_int(gst, POSTPROCESS_SHADER, U_SSAO_ENABLED, &gst->ssao_enabled);
    shader_setu_int(gst, POSTPROCESS_SHADER, U_ANYGUI_OPEN, &gst->player.any_gui_open);
    shader_setu_int(gst, SSAO_SHADER, U_SSAO_KERNEL_SAMPLES, &gst->cfg.ssao_kernel_samples);  


    state_update_shadow_map_uniforms(gst, DEFAULT_SHADER);
    state_update_shadow_map_uniforms(gst, FOLIAGE_SHADER);
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

        //update_enemy_spawn_systems(gst); 
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
    update_npc(gst, &gst->npc);
    player_update(gst, &gst->player);
    state_update_shadow_cams(gst);

    if(!gst->xp_update_done) {
        
        gst->xp_update_timer += gst->dt;
        if(gst->xp_update_timer >= 0.001) {
           
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
}

void state_update_shadow_cams(struct state_t* gst) {
   
    for(int i = 0; i < MAX_SHADOW_LEVELS; i++) {
        Camera* cam = &gst->shadow_cams[i];
        cam->target = (Vector3){0, 0, 0};
        cam->target.x =     (gst->player.cam.position.x - cam->target.x);
        cam->target.z = 1.0+(gst->player.cam.position.z - cam->target.z);
        cam->position = gst->player.cam.position;
        cam->position.y += gst->shadow_cam_height;

    }
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

    float top_color4f[4] = {
        (float)fog->color_top.r / 255.0,
        (float)fog->color_top.g / 255.0,
        (float)fog->color_top.b / 255.0,
        1.0
    };

    float bottom_color4f[4] = {
        (float)fog->color_bottom.r / 255.0,
        (float)fog->color_bottom.g / 255.0,
        (float)fog->color_bottom.b / 255.0,
        1.0
    };

    float settings[4] = { 0 };


    if(fog->mode == FOG_MODE_TORENDERDIST) {
        float test = 1.0 / (gst->render_dist-gst->render_dist/2.0);
        test = pow(test, exp(test));
        settings[0] = test;
    }




    printf("'%s': New fog density = %f\n", __func__, settings[0]);

    size_t offset;
    size_t size = sizeof(float)*4;

    offset = 0;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, settings);

    offset = 16;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, top_color4f);

    offset = 32;
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, bottom_color4f);

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
    gst->terrain.waterplane.transform = MatrixScale(1, 1.0, 1);

    if(gst->fog.mode == FOG_MODE_TORENDERDIST) {
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

