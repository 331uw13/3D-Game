
#include "state_render.h"
#include "state.h"
#include "../util.h"

#include <rlgl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/glad.h"

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
    
    int rp_shader_i = DEFAULT_SHADER;
    int rp_shader_skybox_i = SKY_SHADER;
    int rp_shader_foliage_i = FOLIAGE_SHADER;
    int rp_shader_foliage_wind_i = FOLIAGE_WIND_SHADER;

    if((renderpass == RENDERPASS_GBUFFER) || (renderpass == RENDERPASS_SHADOWS)) {
        rp_shader_i = GBUFFER_SHADER;
        rp_shader_foliage_i = GBUFFER_INSTANCE_SHADER;
        rp_shader_foliage_wind_i = GBUFFER_FOLIAGE_WIND_SHADER;
        rp_shader_skybox_i = GBUFFER_SHADER;
    }

    // Prepare enemies.
    for(int i = 0; i < MAX_ENEMY_MODELS; i++) {
        for(int k = 0; k < gst->enemy_models[i].materialCount; k++) {
            gst->enemy_models[i].materials[0].shader = gst->shaders[rp_shader_i];
        }
    }

    // Prepare biomes
    for(int i = 0; i < MAX_BIOME_TYPES; i++) {
        gst->terrain.biome_materials[i].shader = gst->shaders[rp_shader_i];
    }

    // Prepare foliage
    for(size_t i = 0; i < MAX_FOLIAGE_TYPES; i++) {
        Model* fmodel = &gst->terrain.foliage_models[i];
        for(int mi = 0; mi < fmodel->materialCount; mi++) {
            fmodel->materials[mi].shader = gst->shaders[rp_shader_foliage_i];
        }
    }

    // Set wind shader for tree leafs.
    // The shader just manipulates vertices with voronoise.
    gst->terrain.foliage_models[TF_COMFY_TREE_0].materials[1].shader = gst->shaders[rp_shader_foliage_wind_i];
    gst->terrain.foliage_models[TF_COMFY_TREE_1].materials[1].shader = gst->shaders[rp_shader_foliage_wind_i];


    // Prepare items
    for(int i = 0; i < MAX_ITEM_TYPES; i++) {
        gst->item_models[i].materials[0].shader = gst->shaders[rp_shader_i];
    }

    // Prepare weapon models.
    for(int i = 0; i < MAX_WEAPON_MODELS; i++) {
        gst->weapon_models[i].model.materials[0].shader = gst->shaders[rp_shader_i];
    }
   
    // Prepare "skybox"
    gst->skybox.materials[0].shader = gst->shaders[rp_shader_skybox_i];

    // Prepare npc
    gst->npc.model.materials[0].shader = gst->shaders[rp_shader_i];
    gst->npc.model.materials[1].shader = gst->shaders[rp_shader_i];
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
        render_enemy(gst, ent);
    }

    /*
    int terrain_render_setting = 
        (renderpass == RENDERPASS_SHADOWS) ?
            RENDER_TERRAIN_FOR_SHADOWS : RENDER_TERRAIN_FOR_PLAYER;
    */
    render_terrain(gst, &gst->terrain, renderpass);
    render_npc(gst, &gst->npc);
    render_player(gst, &gst->player);

    // Render chunks items.
    for(int i = 0; i < gst->terrain.num_rendered_chunks; i++) {
        chunk_render_items(gst, gst->terrain.rendered_chunks[i]);
    }
}


static void render_debug_lines(struct state_t* gst) {

    glLineWidth(2.0);

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

    // Chunk borders.
    for(size_t i = 0; i < gst->terrain.num_chunks; i++) {
        struct chunk_t* chunk = &gst->terrain.chunks[i];
        if(chunk->dst2player > gst->render_dist) { continue; }
        render_chunk_borders(gst, chunk, RED);
    }

    
    struct chunk_t* player_chunk = find_chunk(gst, gst->player.position);
    render_chunk_borders(gst, player_chunk, GREEN);


    // Draw player hitboxes.
    /*
    Color hitbox_colors[] = {
        RED,
        YELLOW,
        GREEN,
    };
    for(int i = 0; i < MAX_HITBOXES; i++) {
        DrawCubeWiresV(
                Vector3Add(gst->player.position, gst->player.hitboxes[i].offset),
                gst->player.hitboxes[i].size,
                hitbox_colors[i]
                );
    }
    */
        
    
    glLineWidth(1.0);
}


/*
static int branch_counter = 0;

#define NUM_BRANCHES 2
#define N_INCREMENT ((2*M_PI)/(float)NUM_BRANCHES)

#define POSITIVE 1.0
#define NEGATIVE -1.0

static struct line_t lines[1024*15] = { 0 };
static size_t num_lines = 0;
static float branch_N = 0.0;

void branch(struct state_t* gst, Matrix mtx, float height, int depth, Vector3 rotation) {

    Vector3 p0 = (Vector3) {
        mtx.m12, mtx.m13, mtx.m14
    };

    Matrix offset = MatrixTranslate(0, height, 0);
    Matrix rotm = MatrixRotateXYZ(rotation);
    mtx = MatrixMultiply(offset, mtx);
    mtx = MatrixMultiply(rotm, mtx);

    Vector3 p1 = (Vector3) { 0, 0, 0 };
    p1 = Vector3Transform(p1, mtx);


    Color color = ColorLerp(
        (Color){ 120, 50, 10, 255},
        (Color){ 20, 80, 120, 255},
        pow(normalize(depth, 0, 12), 0.7)
    );

    DrawLine3D(
            p0, p1,
            color
            );
    
    if(depth > 0) {

        height *= gst->fractal_height;

        Vector3 rot = (Vector3) {
            gst->fractal_rx,
            gst->fractal_ry,
            gst->fractal_rz
        };

        branch(gst, mtx, height, depth-1,  rot);
        branch(gst, mtx, height, depth-1,  Vector3Negate(rot));
 

        // TODO: Something for this.
        rot = (Vector3) {
            gst->fractal_rx,
            gst->fractal_ry + 1.57,
            gst->fractal_rz
        };

        branch(gst, mtx, height, depth-1,  rot);
        branch(gst, mtx, height, depth-1,  Vector3Negate(rot));
    }


}


// Generate fractal.
void fractal_tree_test(struct state_t* gst) {
    
    glLineWidth(1.5);
    

    Vector3 p = gst->player.spawn_point;
    p.y = raycast_terrain(&gst->terrain, p.x, p.z).point.y;

    branch_counter = 0;
    branch_N = 0;

    p.y -= 20.0;

    int depth = 8;
    branch(gst, MatrixTranslate(p.x, p.y, p.z), 20, depth, (Vector3){0});

 
}
*/


void state_render(struct state_t* gst) {


    // ------ Shadow map.
    // TODO: This should be rendered in low resolution and then add blur.

    for(int i = 0; i < MAX_SHADOW_LEVELS; i++) {
        struct gbuffer_t* gbuf = &gst->shadow_gbuffers[i];
        rlEnableFramebuffer(gbuf->framebuffer);
        rlClearColor(0, 0, 0, 0);
        rlClearScreenBuffers(); // Clear color and depth.
        rlDisableColorBlend();
        rlViewport(0, 0, gbuf->res_x, gbuf->res_y);
        rlSetFramebufferWidth(gbuf->res_x);
        rlSetFramebufferHeight(gbuf->res_y);
        BeginMode3D(gst->shadow_cams[i]);
        {
            render_scene(gst, RENDERPASS_SHADOWS);
        }
        EndMode3D();
        rlDisableFramebuffer();
        rlClearScreenBuffers();
        rlEnableColorBlend();
    }

    // ------ Geometry data.

    if(gst->ssao_enabled && !gst->player.any_gui_open) {
        rlEnableFramebuffer(gst->gbuffer.framebuffer);
        rlClearColor(0, 0, 0, 0);
        rlClearScreenBuffers(); // Clear color and depth.
        rlDisableColorBlend();
        rlViewport(0, 0, gst->gbuffer.res_x, gst->gbuffer.res_y);
        rlSetFramebufferWidth(gst->gbuffer.res_x);
        rlSetFramebufferHeight(gst->gbuffer.res_y);
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
        gst->cam_proj_matrix = GetCameraProjectionMatrix(&gst->player.cam, (float)gst->res_x / (float)gst->res_y);

        
        // Render debug info if needed. --------
        if(gst->debug) {
            render_debug_lines(gst);
        }

        // ------------

       
        render_scene(gst, RENDERPASS_RESULT);
    

        float psys_render_timestart = GetTime();

        // Particle systems. (rendered only if needed)
        {
            // Environment
            render_psystem(gst, &gst->psystems[FOG_EFFECT_PSYS], (Color){ 50, 50, 50, 255});
            render_psystem(gst, &gst->psystems[EXPLOSION_PSYS], (Color){ 255, 50, 10, 255});
            render_psystem(gst, &gst->psystems[CLOUD_PSYS], (Color){ 35, 40, 50, 240 });
            render_psystem(gst, &gst->psystems[PRJ_TRAIL_PSYS], (Color){0});
 
            render_psystem(gst, &gst->psystems[PLAYER_WEAPON_PSYS], (Color){0});
            render_psystem(gst, &gst->psystems[ENEMY_WEAPON_PSYS], (Color){0});

            render_psystem(gst, &gst->psystems[PLAYER_HIT_PSYS], (Color){ 255, 20, 20, 255});
            render_psystem(gst, &gst->psystems[ENEMY_HIT_PSYS], (Color){ 255, 120, 20, 255});
        
            render_psystem(gst, &gst->psystems[ENEMY_GUNFX_PSYS], (Color){0});
        
            // Projectile environment hit also needs to be rendered after grass.
            render_psystem(gst, &gst->psystems[PROJECTILE_ENVHIT_PSYS], (Color){0});
        }

        state_timebuf_add(gst, 
                TIMEBUF_ELEM_PSYSTEMS_R,
                GetTime() - psys_render_timestart);
            

        render_player_gunfx(gst, &gst->player);



        /*
        Color cube_color = (Color){ 0, 0, 0, 255 };
        rainbow_palette(sin(gst->time), &cube_color.r, &cube_color.g, &cube_color.b);
        DrawCubeV(test_cube_pos, (Vector3){ 30, 30, 30 }, cube_color);

        DrawSphere(test_sphere_pos, 10.0, (Color){ 80, 255, 80, 255 });

        RayCollision ray = raycast_terrain(&gst->terrain, test_cube_pos.x, test_cube_pos.z);
        test_cube_pos.y = ray.point.y;

        if(IsKeyDown(KEY_LEFT_CONTROL)) {
            test_cube_pos = gst->player.cam.position;
        }
        if(IsKeyDown(KEY_X)) {
            test_sphere_pos = gst->player.cam.position;
        }
        */

    }
    EndMode3D();
    EndTextureMode();
 
    BeginTextureMode(gst->inv_render_target);
    ClearBackground((Color){ 0, 0, 0, 0 });
    BeginMode3D(gst->player.cam);
    {
        if(gst->player.inventory.open) {
            inventory_render(gst, &gst->player.inventory);
        }
    }
    EndMode3D();
    EndTextureMode();
   

    // Get bloom treshold.

    resample_texture(gst,
            gst->bloomtresh_target, // destination.
            gst->env_render_target, // source.
            gst->env_render_target.texture.width,
            gst->env_render_target.texture.height,
            gst->bloomtresh_target.texture.width,
            gst->bloomtresh_target.texture.height,
            BLOOM_TRESHOLD_SHADER
            );
    
    // Downsample bloom treshold
    resample_texture(gst,
            gst->bloom_downsamples[0], // destination.
            gst->bloomtresh_target, // source.
            gst->bloomtresh_target.texture.width,
           -gst->bloomtresh_target.texture.height,
            gst->bloom_downsamples[0].texture.width,
            gst->bloom_downsamples[0].texture.height,
            BLOOM_TRESHOLD_SHADER
            );


    // Downsample more and apply blur.

    Vector2 size = (Vector2){ 
        gst->bloom_downsamples[1].texture.width,
        gst->bloom_downsamples[1].texture.height
    };
    shader_setu_vec2(gst, BLOOM_BLUR_SHADER, U_SCREEN_SIZE, &size);
    
    resample_texture(
            gst,
            gst->bloom_downsamples[1],
            gst->bloom_downsamples[0],
            gst->bloom_downsamples[0].texture.width,
           -gst->bloom_downsamples[0].texture.height,
            gst->bloom_downsamples[1].texture.width,
            gst->bloom_downsamples[1].texture.height,
            BLOOM_BLUR_SHADER
            );


    // Upsample blurred bloom treshold.


    resample_texture(gst,
            gst->bloomtresh_target,
            gst->bloom_downsamples[1],
            gst->bloom_downsamples[1].texture.width,
            gst->bloom_downsamples[1].texture.height,
            gst->bloomtresh_target.texture.width,
            gst->bloomtresh_target.texture.height,
            -1 // No special shader needed.
            );



    // TODO: Move ssao to its own functions.
    if(!gst->ssao_enabled || gst->player.any_gui_open) {
        return;
    }

    // SSAO.

    // NOTE: This still needs ALOT of work to make it look good.

    // Downsample environment render target for ssao.
    // To save frame time ssao can be done in lower resolution.

    resample_texture(gst,
            gst->env_render_downsample,
            gst->env_render_target,
            gst->res_x,
            gst->res_y,
            gst->gbuffer.res_x,
            gst->gbuffer.res_y,
            -1
            );
   
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
        // Bloomtreshold texture is needed for the ssao because otherwise it could have ssao on top of very bright objects
        // and that would not make much sense.
        shader_setu_sampler(gst, SSAO_SHADER, U_BLOOMTRESH_TEX, gst->bloomtresh_target.texture.id);
        
        shader_setu_matrix(gst, SSAO_SHADER, U_CAMVIEW_MATRIX, gst->cam_view_matrix);
        shader_setu_matrix(gst, SSAO_SHADER, U_CAMPROJ_MATRIX, gst->cam_proj_matrix);
  
        for(int i = 0; i < gst->cfg.ssao_kernel_samples; i++) {
            SetShaderValueV(ssao_shader,
                    GetShaderLocation(ssao_shader, TextFormat("ssao_kernel[%i]",i)),
                    &gst->ssao_kernel[i], SHADER_UNIFORM_VEC3, 1); 
        }

        DrawTexturePro(gst->env_render_downsample.texture,
                    (Rectangle){
                        0, 0, gst->ssao_target.texture.width, -gst->ssao_target.texture.height
                    },
                    (Rectangle){
                        0, 0, gst->ssao_target.texture.width, -gst->ssao_target.texture.height
                    },
                    (Vector2){0}, 0.0, WHITE
                    );
    }
    EndShaderMode();
    EndTextureMode();


    // Upsample ssao and apply blur.

    Vector2 screen_size = (Vector2){ gst->res_x, gst->res_y };
    shader_setu_sampler(gst, SSAO_BLUR_SHADER, U_GBUFPOS_TEX, gst->gbuffer.position_tex);
    shader_setu_vec2(gst, SSAO_BLUR_SHADER, U_SCREEN_SIZE, &screen_size);

    resample_texture(gst,
            gst->ssao_final,
            gst->ssao_target, 
            gst->gbuffer.res_x,
            gst->gbuffer.res_y,
            gst->res_x,
            gst->res_y,
            SSAO_BLUR_SHADER);

}

