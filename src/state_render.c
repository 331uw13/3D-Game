
#include "state_render.h"
#include "state.h"
#include "util.h"

#include <rlgl.h>
#include <stdio.h>
#include <stdlib.h>

static void set_enemies_render_shader(struct state_t* gst, int shader_index) {
    for(int i = 0; i < MAX_ENEMY_MODELS; i++) {
        for(int k = 0; k < gst->enemy_models[i].materialCount; k++) {
            gst->enemy_models[i].materials[0].shader = gst->shaders[shader_index];
        }
    }
}

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

#define RENDERPASS_FINAL 0
#define RENDERPASS_GBUFFER 1

static void render_terrain_and_enemies(struct state_t* gst, int renderpass) {
    int terrain_shader_index = DEFAULT_SHADER;
    int foliage_shader_index = FOLIAGE_SHADER;
    int enemy_shader_index = DEFAULT_SHADER;


    if(renderpass == RENDERPASS_GBUFFER) {
        terrain_shader_index = GBUFFER_SHADER;
        enemy_shader_index = GBUFFER_SHADER;
        foliage_shader_index = GBUFFER_INSTANCE_SHADER;
    }

    /*
    switch(renderpass) {
        
        case RENDERPASS_DEPTH_DATA:
            terrain_shader_index = WDEPTH_SHADER;
            foliage_shader_index = WDEPTH_INSTANCE_SHADER;
            enemy_shader_index = WDEPTH_SHADER;
            break;

   
            // ...

        default:break;
    }
    */

   
    set_enemies_render_shader(gst, enemy_shader_index);
    
    // Enemies.
    for(size_t i = 0; i < gst->num_enemies; i++) {
        struct enemy_t* ent = &gst->enemies[i];
        if(!ent->alive) {
            continue;
        }

        render_enemy(gst, ent);
    }
    // Terrain.
    render_terrain(gst, &gst->terrain, terrain_shader_index, foliage_shader_index);
}



void state_render(struct state_t* gst) {

    
    // ------ Geometry data.

    rlEnableFramebuffer(gst->gbuffer.framebuffer);    
    rlClearColor(0, 0, 0, 0);
    rlClearScreenBuffers(); // Clear color and depth.
    rlDisableColorBlend();
    BeginMode3D(gst->player.cam);
    {
        rlEnableShader(gst->shaders[GBUFFER_SHADER].id);
        render_terrain_and_enemies(gst, RENDERPASS_GBUFFER);
    }
    EndMode3D();
    rlDisableFramebuffer();
    rlClearScreenBuffers();
    rlEnableColorBlend();

   
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

        render_terrain_and_enemies(gst, RENDERPASS_FINAL);

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

        player_render(gst, &gst->player);
        render_items(gst);

        //_state_render_crithit_markers(gst);

    }
    EndMode3D();
    EndTextureMode();
    
    // Get bloom treshold texture.

    
    BeginTextureMode(gst->bloomtresh_target);
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
