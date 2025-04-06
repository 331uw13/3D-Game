#include <raylib.h>
#include <stdio.h>
#include <math.h>
#include <raymath.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "state.h"
#include "input.h"
#include "util.h"
#include "terrain.h"
#include "state_render.h"

#include <rlgl.h>



void cleanup(struct state_t* gst);

void loop(struct state_t* gst) {

    while(!WindowShouldClose() && gst->running) {

        gst->dt = GetFrameTime();
        gst->time = GetTime();

        if(IsWindowResized()) {
            printf("\033[31m(WARNING!): Resizing window is not yet supported in runtime."
                    " Change it in config file.\n (Also the config file doesnt exists yet)\033[0m\n");
        }

      
        state_update_frame(gst);
        state_update_shader_uniforms(gst);
        state_render(gst);



        BeginDrawing();
        {
            ClearBackground(BLACK);



            // Finally post process everything.
            //
            BeginShaderMode(gst->shaders[POSTPROCESS_SHADER]);
            {
                rlEnableShader(gst->shaders[POSTPROCESS_SHADER].id);
                shader_setu_sampler(gst, POSTPROCESS_SHADER, U_BLOOMTRESH_TEX, gst->bloomtresh_target.texture.id);

                SetShaderValueTexture(gst->shaders[POSTPROCESS_SHADER],
                        GetShaderLocation(gst->shaders[POSTPROCESS_SHADER],
                            "ssao_texture"), gst->ssao_final.texture);
    

                DrawTexturePro(gst->env_render_target.texture,
                        (Rectangle){
                            0, 0, gst->env_render_target.texture.width, -gst->env_render_target.texture.height
                        },
                        (Rectangle){
                            //0, 0, gst->res_x, gst->res_y
                            0, 0, gst->screen_size.x, gst->screen_size.y
                        },
                        (Vector2){0}, 0.0, WHITE
                        ); 
            }
            EndShaderMode();


            render_inventory(gst, &gst->player);

            // Draw Crosshair if player is aiming.
            if(gst->player.is_aiming) {
                int center_x = GetScreenWidth() / 2;
                int center_y = GetScreenHeight() / 2;
                DrawPixel(center_x, center_y, WHITE);
           
                DrawPixel(center_x-1, center_y, GRAY);
                DrawPixel(center_x, center_y-1, GRAY);
                DrawPixel(center_x+1, center_y, GRAY);
                DrawPixel(center_x, center_y+1, GRAY);
          
                DrawPixel(center_x-2, center_y, GRAY);
                DrawPixel(center_x, center_y-2, GRAY);
                DrawPixel(center_x+2, center_y, GRAY);
                DrawPixel(center_x, center_y+2, GRAY);
            }

            if(gst->menu_open) {
                gui_render_menu_screen(gst);
            }
            else
            if(gst->player.powerup_shop.open) {
                gui_render_powerup_shop(gst);
            }

            if(!gst->menu_open && !gst->player.powerup_shop.open) {
                render_player_stats(gst, &gst->player);
            }

            if(gst->devmenu_open) {
                gui_render_devmenu(gst);
            }
            
            if(gst->player.item_in_crosshair && !gst->player.inventory.open) {
                struct item_t* item = gst->player.item_in_crosshair;

                Vector2 item_name_pos = (Vector2) {
                    gst->res_x/2 - item->name_width / 2 - 200,
                    gst->res_y/2 + 100,
                };

                DrawText(item->name, item_name_pos.x, item_name_pos.y, 20, WHITE);
                DrawText("< Press (F) to pickup >\0", 
                        item_name_pos.x, item_name_pos.y+30, 20,
                        (Color){ 100, 100, 100, 255 });

                if(IsKeyPressed(KEY_F)) {
                    item->enabled = !inv_add_item(gst, &gst->player, item);
                }
            }


            if(gst->debug) {
                int next_y = 50;
                int dtext_x = gst->res_x - 500;
                const int y_inc = 25;


                DrawText(TextFormat("NumVisibleChunks: %i / %li", gst->terrain.num_visible_chunks, gst->terrain.num_chunks),
                        dtext_x, next_y, 20, PURPLE);
                next_y += y_inc;

                DrawText(TextFormat("NumRenderedEnemies: %li / %li", gst->num_enemies_rendered, gst->num_enemies),
                        dtext_x, next_y, 20, PURPLE);
                next_y += y_inc;

                DrawText(TextFormat("X=%0.3f", gst->player.cam.position.x),
                        dtext_x, next_y, 20, (Color){ 150, 80, 200, 255 });
                next_y += y_inc;
                DrawText(TextFormat("Y=%0.3f", gst->player.cam.position.y),
                        dtext_x, next_y, 20, (Color){ 150, 80, 200, 255 });
                next_y += y_inc;
                DrawText(TextFormat("Z=%0.3f", gst->player.cam.position.z),
                        dtext_x, next_y, 20, (Color){ 150, 80, 200, 255 });
                next_y += y_inc;
                
                DrawText("(Debug ON)", gst->res_x - 200, 10, 20, GREEN);
            }


            DrawText(TextFormat("FPS: %i", GetFPS()),
                    gst->res_x - 100, gst->res_y-30, 20, WHITE);


        }
        EndDrawing();
    }
}



void cleanup(struct state_t* gst) {
    
   
    delete_terrain(&gst->terrain);
    delete_player(&gst->player);
    delete_prjmods(gst);
    state_delete_all_textures(gst);
    state_delete_all_shaders(gst);
    state_delete_all_psystems(gst);
    state_delete_all_sounds(gst);
    state_delete_all_enemy_models(gst);
    state_delete_all_item_models(gst);

    UnloadTexture(gst->ssao_noise_tex);
    UnloadRenderTexture(gst->env_render_target);
    UnloadRenderTexture(gst->env_render_downsample);
    UnloadRenderTexture(gst->bloomtresh_target);
    UnloadRenderTexture(gst->ssao_target);
    UnloadRenderTexture(gst->ssao_final);
    state_delete_gbuffer(gst);

    UnloadRenderTexture(gst->gbuf_pos_up);
    UnloadRenderTexture(gst->gbuf_norm_up);
    UnloadRenderTexture(gst->gbuf_depth_up);
    delete_npc(&gst->npc);

    for(int i = 0; i < NUM_BLOOM_DOWNSAMPLES; i++) {
        UnloadRenderTexture(gst->bloom_downsamples[i]);
    }

    UnloadModel(gst->skybox);

    for(int i = 0; i < MAX_UBOS; i++) {
        glDeleteBuffers(1, &gst->ubo[i]);
    }

    free(gst->ssao_kernel);
    UnloadFont(gst->font);

    printf("\033[35m -> Cleanup done...\033[0m\n");
    CloseWindow();
}

#include "platform/platform.h"

#define CFGBUF_SIZE 64

int cfgbool_to_int(char* buf) {
    return CLAMP(strcmp(buf, "false"), 0, 1);
}

int read_config(struct state_t* gst) {
    int result = 0;

    platform_file_t cfgfile;
    platform_init_file(&cfgfile);

    if(!platform_read_file(&cfgfile, "game.cfg")) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Failed to read config file\033[0m\n",
                __func__);
        goto error;
    }

    char buf[CFGBUF_SIZE] = { 0 };
    
    // Resolution.
    int res_found = 0;
    if((res_found = read_cfgvar(&cfgfile, "resolution_x", buf, CFGBUF_SIZE))) {
        int res_x = atoi(buf);
        if(res_x > 0) {
            gst->cfg.resolution_x = res_x;
        }
        res_found = 1;
    }

    if((res_found = read_cfgvar(&cfgfile, "resolution_y", buf, CFGBUF_SIZE))) {
        int res_y = atoi(buf);
        if(res_y > 0) {
            gst->cfg.resolution_y = res_y;
        }
    }

    if(!res_found) {
        fprintf(stderr, "\033[35m(WARNING) '%s': Something went wrong while reading resolution from config file\n"
                " using default resoltion now.\033[0m",__func__);
        gst->cfg.resolution_x = DEFAULT_RES_X;
        gst->cfg.resolution_y = DEFAULT_RES_Y;
    }


    // Fullscreen?

    if(read_cfgvar(&cfgfile, "fullscreen", buf, CFGBUF_SIZE)) {
        gst->cfg.fullscreen = cfgbool_to_int(buf);
    }
    
    // ssao quality.
    read_cfgvar(&cfgfile, "ssao_quality", buf, CFGBUF_SIZE);

    if(!strcmp(buf, "low")) {
        gst->cfg.ssao_quality = CFG_SSAO_QLOW;
        printf("'%s': Using low ssao quality\n", __func__);
    }
    else
    if(!strcmp(buf, "medium")) {
        gst->cfg.ssao_quality = CFG_SSAO_QMED;
        printf("'%s': Using medium ssao quality\n", __func__);
    }
    else
    if(!strcmp(buf, "high")) {
        gst->cfg.ssao_quality = CFG_SSAO_QHIGH;
        printf("'%s': Using high ssao quality\n", __func__);
    }
    else {
        gst->cfg.ssao_quality = CFG_SSAO_QLOW;
        fprintf(stderr, "\033[35m(WARNING) '%s': Something went wrong while reading ssao quality from config file\n"
                " using low quality now\033[0m\n", __func__);
    }

    if(read_cfgvar(&cfgfile, "ssao_kernel_samples", buf, CFGBUF_SIZE)) {
        int num_samples = CLAMP(atoi(buf), 8, MAX_SSAO_KERNEL_SIZE);
        if(num_samples > 0 && num_samples) {
            gst->cfg.ssao_kernel_samples = num_samples;
            printf("'%s': Using %i ssao kernel samples\n", __func__, num_samples);
        }
    }
    else {
        gst->cfg.ssao_kernel_samples = DEFAULT_SSAO_KERNEL_SAMPLES;
        fprintf(stderr, "\033[35m(WARNING) '%s': Something went wrong while reading ssao kernel samples count"
                " using %i now\033[0m\n", __func__, DEFAULT_SSAO_KERNEL_SAMPLES);
    }
   
    // ssao enabled?
    if(read_cfgvar(&cfgfile, "ssao_enabled", buf, CFGBUF_SIZE)) {
        gst->ssao_enabled = cfgbool_to_int(buf);
        printf("%i\n", gst->ssao_enabled);
    }

    result = 1;
    platform_close_file(&cfgfile);

error:
    return result;
}


void first_setup(struct state_t* gst) {

    if(!read_config(gst)) {
        return;
    }

    InitWindow(gst->cfg.resolution_x, gst->cfg.resolution_y, "3D-Game");
    SetExitKey(-1);
    gst->font = LoadFont("res/Topaz-8.ttf");

    DisableCursor();
    SetTargetFPS(TARGET_FPS);
    SetTraceLogLevel(LOG_ERROR);

    gst->screen_size = (Vector2){ GetScreenWidth(), GetScreenHeight() };
    gst->res_x = (int)gst->screen_size.x;
    gst->res_y = (int)gst->screen_size.y;

    gst->ssao_kernel = NULL;
    gst->ssao_kernel = malloc(gst->cfg.ssao_kernel_samples * sizeof *gst->ssao_kernel);

    gst->num_textures = 0;
    gst->num_textures = 0;
    gst->debug = 0;
    gst->num_enemies = 0;
    gst->num_prj_lights = 0;
    gst->dt = 0.016;
    gst->time = 0.0;
    gst->num_enemy_weapons = 0;
    gst->menu_open = 0;
    gst->running = 1;
    gst->xp_update_done = 1;
    memset(gst->enemies, 0, MAX_ALL_ENEMIES * sizeof *gst->enemies);

    init_shaderutil(gst);


    const float terrain_scale = 20.0;
    const u32   terrain_size = 1024;
    const float terrain_amplitude = 30.0;
    const float terrain_pnfrequency = 30.0;
    const int   terrain_octaves = 3;
    /*
    const float terrain_scale = 20.0;
    const u32   terrain_size = 2048;
    const float terrain_amplitude = 30.0;
    const float terrain_pnfrequency = 80.0;
    const int   terrain_octaves = 3;
    */
    /*
    gst->num_crithit_markers = 0;
    gst->crithit_marker_maxlifetime = 1.5;

    memset(gst->crithit_markers, 0, MAX_RENDER_CRITHITS * sizeof *gst->crithit_markers);
    */


    state_create_ubo(gst, LIGHTS_UBO,    2, MAX_NORMAL_LIGHTS * LIGHT_UB_STRUCT_SIZE);
    state_create_ubo(gst, PRJLIGHTS_UBO, 3, MAX_PROJECTILE_LIGHTS * LIGHT_UB_STRUCT_SIZE);
    state_create_ubo(gst, FOG_UBO,       4, FOG_UB_STRUCT_SIZE);


    state_setup_all_textures(gst);
    state_setup_all_shaders(gst);
    state_setup_all_weapons(gst);
    state_setup_all_sounds(gst);
    state_setup_all_enemy_models(gst);
    state_setup_all_item_models(gst);
    
    printf("Screen size: %0.0fx%0.0f\n", gst->screen_size.x, gst->screen_size.y);
    
    state_setup_gbuffer(gst);
    state_setup_ssao(gst);
    state_setup_render_targets(gst);
    setup_npc(gst, &gst->npc);

    gst->skybox = LoadModelFromMesh(GenMeshSphere(1.0, 32, 32));
    gst->skybox.materials[0] = LoadMaterialDefault();


    // (!IMPORTANT!)
    // TODO: Errors from all functions on init should be checked!
    //       alot of memory may be leaked if crash after terrain was created.


    // --- Setup Terrain ----
    {
        init_perlin_noise();
        gst->terrain = (struct terrain_t) { 0 };

        const int terrain_seed = GetRandomValue(0, 9999999);
        //const int terrain_seed = 2010357;//GetRandomValue(0, 9999999);

        printf("(INFO) '%s': Terrain seed = %i\n",
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

    }

    init_player_struct(gst, &gst->player);
    state_setup_all_psystems(gst);
    
    setup_natural_item_spawn_settings(gst);
    setup_default_enemy_spawn_settings(gst);

    set_powerup_defaults(gst, &gst->player.powerup_shop);
    update_powerup_shop_offers(gst);



    int seed = time(0);
    gst->rseed = seed;
    SetRandomSeed(seed);


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

    gst->next_explosion_light_index = MAX_STATIC_LIGHTS;


    for(size_t i = 0; i < MAX_ALL_ENEMIES; i++) {
        gst->enemies[i].modelptr = NULL;
        gst->enemies[i].alive = 0;
        gst->enemies[i].enabled = 0;
    }


    struct light_t SUN = (struct light_t) {
        .type = LIGHT_DIRECTIONAL,
        .enabled = 1,
        .color = (Color){ 240, 210, 200, 255 },
        .position = (Vector3){ -1, 1, -1 },
        .strength = 0.2,
        .index = SUN_LIGHT_ID
    };

    gst->player.gun_light = (struct light_t) {
        .type = LIGHT_POINT,
        .enabled = 1,
        .strength = 0.75,
        .radius = 2.0,
        .index = PLAYER_GUN_LIGHT_ID
    }; // Player's gun light is updated from 'src/player.c'


    // Setup fog.
    gst->fog = (struct fog_t) {
        .mode = FOG_MODE_TORENDERDIST,
        .color_top = (Color){ 5, 0, 10 },
        .color_bottom = (Color){ 60, 30, 80 },
        .density = 0.0 // Density is ignored when fog mode is TORENDERDIST
    };




    set_light(gst, &SUN, LIGHTS_UBO);
    set_fog_settings(gst, &gst->fog);
}

int main(void) {

    struct state_t* gst = NULL;
    gst = malloc(sizeof *gst);

    if(!gst) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Failed to allocate memory for game state.\033[0m\n",
                __func__);
        return 1;
    }

    first_setup(gst);
    loop(gst);
    cleanup(gst);

    free(gst);


    printf("Exit 0.\n");
    return 0;
}

