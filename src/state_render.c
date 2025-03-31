
#include "state_render.h"
#include "state.h"
#include "util.h"

#include <rlgl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
static void set_enemies_render_shader(struct state_t* gst, int shader_index) {
    for(int i = 0; i < MAX_ENEMY_MODELS; i++) {
        for(int k = 0; k < gst->enemy_models[i].materialCount; k++) {
            gst->enemy_models[i].materials[0].shader = gst->shaders[shader_index];
        }
    }
}
*/

/*
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
*/

void prepare_renderpass(struct state_t* gst, int renderpass) {
    
    int rp_shader_i;
    int rp_shader_foliage_i;

    if(renderpass == RENDERPASS_RESULT) {
        rp_shader_i = DEFAULT_SHADER;
        rp_shader_foliage_i = FOLIAGE_SHADER;
    }
    else
    if(renderpass == RENDERPASS_GBUFFER) {
        rp_shader_i = GBUFFER_SHADER;
        rp_shader_foliage_i = GBUFFER_INSTANCE_SHADER;
    }


    // Prepare enemies.
    for(int i = 0; i < MAX_ENEMY_MODELS; i++) {
        for(int k = 0; k < gst->enemy_models[i].materialCount; k++) {
            gst->enemy_models[i].materials[0].shader = gst->shaders[rp_shader_i];
        }
    }


    // Prepare terrain and foliage.
    gst->terrain.material.shader = gst->shaders[rp_shader_i];
    for(size_t i = 0; i < MAX_FOLIAGE_TYPES; i++) {
        Model* fmodel = &gst->terrain.foliage_models[i];
        for(int mi = 0; mi < fmodel->materialCount; mi++) {
            fmodel->materials[mi].shader = gst->shaders[rp_shader_foliage_i];
        }
    }

    // Prepare player.
    gst->player.gunmodel.materials[0].shader = gst->shaders[rp_shader_i];
    gst->player.arms_material.shader = gst->shaders[rp_shader_i];
    gst->player.hands_material.shader = gst->shaders[rp_shader_i];

    
    // Prepare "skybox"
    gst->skybox.materials[0].shader = gst->shaders[rp_shader_i];

    // Prepare items.
    for(size_t i = 0; i < MAX_ITEM_MODELS; i++) {
        gst->item_models[i].materials[0].shader = gst->shaders[rp_shader_i];
    }
}



static void render_scene(struct state_t* gst, int renderpass) {

    prepare_renderpass(gst, renderpass);
    
    // Render huge sphere, 
    // ssao algorithm freaks out if there is nothing behind
    // because it doesnt have valid depth.
    rlDisableDepthMask();
    rlDisableBackfaceCulling();
    DrawModel(gst->skybox, gst->player.cam.position, gst->render_dist, BLACK);
    rlEnableDepthMask();
    rlEnableBackfaceCulling();
    
    // Enemies.
    // TODO: Instanced rendering for enemies.


    gst->num_enemies_rendered = 0;

    for(size_t i = 0; i < gst->num_enemies; i++) {
        struct enemy_t* ent = &gst->enemies[i];
        if(!ent->alive) {
            continue;
        }
        render_enemy(gst, ent);
    }


    render_terrain(gst, &gst->terrain);
    render_player(gst, &gst->player);
    render_items(gst);
}



static Vector3 test_cube_pos = (Vector3){0, 0, 0 };

void state_render(struct state_t* gst) {


    // ------ Geometry data.

    if(gst->ssao_enabled && !gst->player.any_gui_open) {
        rlEnableFramebuffer(gst->gbuffer.framebuffer);    
        rlClearColor(0, 0, 0, 0);
        rlClearScreenBuffers(); // Clear color and depth.
        rlDisableColorBlend();
        BeginMode3D(gst->player.cam);
        {
            rlEnableShader(gst->shaders[GBUFFER_SHADER].id);
            render_scene(gst, RENDERPASS_GBUFFER);
        }
        EndMode3D();
        rlDisableFramebuffer();
        rlClearScreenBuffers();
        rlEnableColorBlend();
    }

   
    //  ------ Final pass.

    BeginTextureMode(gst->env_render_target);
    ClearBackground(gst->render_bg_color);
    BeginMode3D(gst->player.cam);
    {

        // Save camera view and projection matrix for postprocessing.
        gst->cam_view_matrix = GetCameraViewMatrix(&gst->player.cam);
        gst->cam_proj_matrix = GetCameraProjectionMatrix(&gst->player.cam, (float)gst->scrn_w / (float)gst->scrn_h);

        
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

        render_scene(gst, RENDERPASS_RESULT);
        
        // Particle systems. (rendered only if needed)
        {
            render_psystem(gst, &gst->psystems[PLAYER_WEAPON_PSYS], (Color){0});
            render_psystem(gst, &gst->psystems[ENEMY_WEAPON_PSYS], (Color){0});
            render_psystem(gst, &gst->psystems[PROJECTILE_ENVHIT_PSYS], (Color){0});

            render_psystem(gst, &gst->psystems[PLAYER_HIT_PSYS], (Color){ 255, 20, 20, 255});
            render_psystem(gst, &gst->psystems[ENEMY_HIT_PSYS], (Color){ 255, 120, 20, 255});
            render_psystem(gst, &gst->psystems[ENEMY_GUNFX_PSYS], (Color){0});


            // Environment
            render_psystem(gst, &gst->psystems[FOG_EFFECT_PSYS], (Color){ 50, 50, 50, 255});
            render_psystem(gst, &gst->psystems[WATER_SPLASH_PSYS], (Color){ 30, 80, 170, 200});
            render_psystem(gst, &gst->psystems[EXPLOSION_PSYS], (Color){ 255, 50, 10, 255});
            render_psystem(gst, &gst->psystems[CLOUD_PSYS], (Color){ 70, 60, 50, 255 });
            render_psystem(gst, &gst->psystems[PRJ_TRAIL_PSYS], (Color){ 0 });
            
        }

        // Water
        {
            rlDisableBackfaceCulling();
            DrawModel(gst->terrain.waterplane, 
                    (Vector3){gst->player.position.x, gst->terrain.water_ylevel, gst->player.position.z},
                    1.0,
                    (Color){ 255, 255, 255, 255 });
            
            rlEnableBackfaceCulling();
        }

        Color cube_color = (Color){ 0, 0, 0, 255 };
        rainbow_palette(sin(gst->time), &cube_color.r, &cube_color.g, &cube_color.b);
        DrawCubeV(test_cube_pos, (Vector3){ 30, 30, 30 }, cube_color);

        RayCollision ray = raycast_terrain(&gst->terrain, test_cube_pos.x, test_cube_pos.z);
        test_cube_pos.y = ray.point.y;

        if(IsKeyDown(KEY_LEFT_CONTROL)) {
            test_cube_pos = gst->player.cam.position;
        }


        // Player Gun FX
        {

            struct player_t* p = &gst->player;

            if(p->gunfx_timer < 1.0) {
            
 
                p->gunfx_model.transform = p->gunmodel.transform;
                
                p->gunfx_model.transform 
                    = MatrixMultiply(MatrixTranslate(0.28, -0.125, -3.5), p->gunfx_model.transform);
                p->gunfx_model.transform = MatrixMultiply(MatrixRotateX(1.5), p->gunfx_model.transform);

                float st = lerp(p->gunfx_timer, 2.0, 0.0);
                p->gunfx_model.transform = MatrixMultiply(MatrixScale(st, st, st), p->gunfx_model.transform);
                
                shader_setu_color(gst, GUNFX_SHADER, U_GUNFX_COLOR, &gst->player.weapon.color);
                DrawMesh(
                        p->gunfx_model.meshes[0],
                        p->gunfx_model.materials[0],
                        p->gunfx_model.transform
                        );

                p->gunfx_timer += gst->dt*13.0;

            }
        }



        // TODO:
        //render_items(gst);

    }
    EndMode3D();
    EndTextureMode();
    
    // Get bloom treshold texture.
 
    BeginTextureMode(gst->bloomtresh_target);
    ClearBackground((Color){ 0, 0, 0, 255 });
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


    // Down sample and blur bloom treshold.

    BeginTextureMode(gst->bloomtresh_downsample);
    ClearBackground((Color){ 0, 0, 0, 255 });
    BeginShaderMode(gst->shaders[BLOOM_BLUR_SHADER]);
    {
        int src_width = gst->bloomtresh_target.texture.width;
        int src_height = gst->bloomtresh_target.texture.height;

        int dst_width = gst->bloomtresh_downsample.texture.width;
        int dst_height = gst->bloomtresh_downsample.texture.height;

        Vector2 size = (Vector2){ dst_width, dst_height };
        shader_setu_vec2(gst, BLOOM_BLUR_SHADER, U_SCREEN_SIZE, &size);

        DrawTexturePro(gst->bloomtresh_target.texture,
                (Rectangle){ 0, 0, src_width, src_height },
                (Rectangle){ 0, 0, dst_width, dst_height },
                (Vector2){0}, 0.0, WHITE
                );
    }
    EndShaderMode();
    EndTextureMode();
    

    // Upscale blurred bloom treshold.

    BeginTextureMode(gst->bloomtresh_target);
    ClearBackground((Color){ 0, 0, 0, 255 });
    {
        int dst_width = gst->bloomtresh_target.texture.width;
        int dst_height = gst->bloomtresh_target.texture.height;

        int src_width = gst->bloomtresh_downsample.texture.width;
        int src_height = gst->bloomtresh_downsample.texture.height;

        DrawTexturePro(gst->bloomtresh_downsample.texture,
                (Rectangle){ 0, 0, src_width, src_height },
                (Rectangle){ 0, 0, dst_width, dst_height },
                (Vector2){0}, 0.0, WHITE
                );
    }
    EndTextureMode();




    if(!gst->ssao_enabled || gst->player.any_gui_open) {
        return;
    }
    // Screen space ambient occlusion.
    
    BeginTextureMode(gst->ssao_target);
    ClearBackground((Color){ 255, 255, 255, 255 });
    BeginShaderMode(gst->shaders[SSAO_SHADER]);
    {
        const Shader ssao_shader = gst->shaders[SSAO_SHADER];
        rlEnableShader(ssao_shader.id);

        shader_setu_sampler(gst, SSAO_SHADER, U_GBUFPOS_TEX, gst->gbuffer.position_tex);
        shader_setu_sampler(gst, SSAO_SHADER, U_GBUFNORM_TEX, gst->gbuffer.normal_tex);
        shader_setu_sampler(gst, SSAO_SHADER, U_GBUFDEPTH_TEX, gst->gbuffer.depth_tex);
        shader_setu_sampler(gst, SSAO_SHADER, U_SSAO_NOISE_TEX, gst->ssao_noise_tex.id);
  
        shader_setu_matrix(gst, SSAO_SHADER, U_CAMVIEW_MATRIX, gst->cam_view_matrix);
        shader_setu_matrix(gst, SSAO_SHADER, U_CAMPROJ_MATRIX, gst->cam_proj_matrix);
  
        for(int i = 0; i < SSAO_KERNEL_SIZE; i++) {
            SetShaderValueV(ssao_shader,
                    GetShaderLocation(ssao_shader, TextFormat("ssao_kernel[%i]",i)),
                    &gst->ssao_kernel[i], SHADER_UNIFORM_VEC3, 1);
        
        }

        // Render into fullscreen rectangle so it can be blurred in postprocessing.
        // to get rid of most noticable artifacts.
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


