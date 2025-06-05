#include <raylib.h>
#include <stdio.h>
#include <math.h>
#include <raymath.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "state/state.h"
#include "state/state_setup.h"
#include "state/state_free.h"
#include "state/state_render.h"
#include "input.h"
#include "util.h"
//#include "terrain.h"

#include <rlgl.h>



void cleanup(struct state_t* gst);


void loop(struct state_t* gst) {


    while(!WindowShouldClose() && gst->running) {
        gst->test_counter = 0;
        gst->dt = GetFrameTime();
        gst->time = GetTime();


        if(IsWindowResized()) {
            printf("\033[31m(WARNING!): Resizing window is not yet supported in runtime."
                    " Change it in config file.\033[0m\n");
        }


        gst->mouse_double_click = 0;
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            
            // Measure the time difference between mouse clicks.
            // if its less than treshold we can know double click happened.

            float click_time_diff = GetTime() - gst->mouse_click_time_point;
            gst->mouse_click_time_point = GetTime();

            if(click_time_diff < 0.25) {
                gst->mouse_double_click = 1;
            }

        }
    
      
        state_update_frame(gst); 
        state_update_shader_uniforms(gst);
        state_render(gst);


        BeginDrawing();
        {

            // Finally post process everything.
            //
            BeginShaderMode(gst->shaders[POSTPROCESS_SHADER]);
            {
                rlEnableShader(gst->shaders[POSTPROCESS_SHADER].id);
                shader_setu_sampler(gst, POSTPROCESS_SHADER, U_BLOOMTRESH_TEX, gst->bloomtresh_target.texture.id);

                SetShaderValueTexture(gst->shaders[POSTPROCESS_SHADER],
                        GetShaderLocation(gst->shaders[POSTPROCESS_SHADER],
                            "ssao_texture"), gst->ssao_final.texture);
                
                SetShaderValueTexture(gst->shaders[POSTPROCESS_SHADER],
                        GetShaderLocation(gst->shaders[POSTPROCESS_SHADER],
                            "inventory_texture"), gst->inv_render_target.texture);
               
                shader_setu_int(gst, POSTPROCESS_SHADER, U_INVENTORY_OPEN, &gst->player.inventory.open);

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

            
            /*
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
            */

            if(gst->player.in_scope_view && !gst->player.any_gui_open) {
                BeginShaderMode(gst->shaders[SCOPE_CROSSHAIR_SHADER]);
                DrawRectangle(0, 0, gst->screen_size.x, gst->screen_size.y, BLACK);
                EndShaderMode();
            }


            if(gst->menu_open) {
                gui_render_menu_screen(gst);
            }
            
            if(!gst->menu_open) {
                render_player_stats(gst, &gst->player);
            }

            if(gst->devmenu_open) {
                gui_render_devmenu(gst);
            }
           
            if(gst->player.inventory.open) {
                gui_render_inventory_controls(gst, &gst->player.inventory);
            }

            if(!gst->player.any_gui_open && gst->crosshair_item_info) {
                render_item_info(gst);
            }
            else {
                gst->item_info_screen_time = 0;
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
 
                DrawText(TextFormat("NumItemsInChunk: %i", find_chunk(gst, gst->player.position)->num_items),
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
                
                RayCollision ray = raycast_terrain(&gst->terrain, gst->player.position.x, gst->player.position.z);
                DrawText(TextFormat("TerrainLevel=%0.3f", ray.point.y),
                        dtext_x, next_y, 20, (Color){ 200, 80, 170, 255 });
                next_y += y_inc;
        
                DrawText(TextFormat("CurrentBiome=%s", get_biome_name_by_id(gst->player.current_biome->id)),
                        dtext_x, next_y, 20, (Color){ 200, 80, 170, 255 });
                next_y += y_inc;

                DrawText(TextFormat("InBiomeShiftArea: %i", playerin_biomeshift_area(gst, &gst->player)),
                        dtext_x, next_y, 20, (Color){ 200, 80, 170, 255 });
                next_y += y_inc;


                DrawText(TextFormat("PSystemsRenderTime: %f", state_average_timebuf(gst, TIMEBUF_ELEM_PSYSTEMS_R)),
                        dtext_x, next_y, 20, (Color){ 80, 120, 170, 255 });
                next_y += y_inc;

                DrawText(TextFormat("TerrainRenderTime: %f", state_average_timebuf(gst, TIMEBUF_ELEM_TERRAIN_R)),
                        dtext_x, next_y, 20, (Color){ 80, 120, 170, 255 });
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
    

    state_free_everything(gst);
    UnloadModel(gst->skybox);

    if(gst->ssao_kernel) {
        free(gst->ssao_kernel);
    }


    printf("\033[35m -> Cleanup done...\033[0m\n");
    
    UnloadFont(gst->font);
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

    // Resolution divisor (see detail from game.cfg)
    if(read_cfgvar(&cfgfile, "resolution_divisor", buf, CFGBUF_SIZE)) {
        gst->cfg.res_div = 0.0;
        gst->cfg.res_div = atof(buf);
        if(gst->cfg.res_div < 1.0) {
            fprintf(stderr, "\033[35m(WARNING) '%s': Invalid resoltion divisor value, setting it to 1.0\033[0m\n",
                    __func__);
            gst->cfg.res_div = 1.0;
        }

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
    }

    // Render distance?
    if(read_cfgvar(&cfgfile, "render_distance", buf, CFGBUF_SIZE)) {
        gst->cfg.render_dist = CLAMP(atoi(buf), MIN_RENDERDIST, MAX_RENDERDIST);
    }

    result = 1;
    platform_close_file(&cfgfile);

error:
    return result;
}

void tracelog_callback(int logLevel, const char *text, va_list args) {
    char buf[256] = { 0 };
    vsnprintf(buf, 256, text, args);
    printf("\033[36m|\033[0m %s\n", buf);
}

void first_setup(struct state_t* gst) {

    if(!read_config(gst)) {
        return;
    }

    gst->skybox = (Model){ 0 };


    SetTraceLogCallback(tracelog_callback);
    InitWindow(gst->cfg.resolution_x, gst->cfg.resolution_y, "3D-Game");
    SetExitKey(-1);
    gst->font = LoadFont("res/Topaz-8.ttf");

    if(gst->cfg.fullscreen) {
        int monitor = GetCurrentMonitor();
        int mon_w = GetMonitorWidth(monitor);
        int mon_h = GetMonitorHeight(monitor);

        printf("'%s': Changing resolution to %ix%i because fullscreen is enabled\n",
                __func__, mon_w, mon_h);

        ToggleBorderlessWindowed();
        SetWindowSize(mon_w, mon_h);
    }


    DisableCursor();
    SetTargetFPS(TARGET_FPS);
    SetTraceLogLevel(LOG_ERROR);

    gst->screen_size = (Vector2){ GetScreenWidth(), GetScreenHeight() };
    gst->res_x = (int)gst->screen_size.x;
    gst->res_y = (int)gst->screen_size.y;

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

    printf("Screen size: %0.0fx%0.0f\n", gst->screen_size.x, gst->screen_size.y);
    

    // (!IMPORTANT!)
    // TODO: Errors from all functions on init should be checked!
    //       alot of memory may be leaked if crash after terrain was created.

    gst->next_explosion_light_index = MAX_STATIC_LIGHTS;



    state_setup_everything(gst);
    setup_npc(gst, &gst->npc);

    gst->skybox = LoadModelFromMesh(GenMeshSphere(1.0, 32, 32));
    gst->skybox.materials[0] = LoadMaterialDefault();

   
    setup_default_enemy_spawn_settings(gst);


    int seed = time(0);
    gst->rseed = seed;
    SetRandomSeed(seed);


    for(size_t i = 0; i < MAX_ALL_ENEMIES; i++) {
        gst->enemies[i].modelptr = NULL;
        gst->enemies[i].alive = 0;
        gst->enemies[i].enabled = 0;
    }


    gst->gamepad.id = -1;
    for(int i = 0; i < 16; i++) {
        if(IsGamepadAvailable(i)) {
            gst->gamepad.id = i;
            break;
        }
    }

    if(gst->gamepad.id >= 0) {
        gst->gamepad.sensetivity = 6.689830;
        printf("\033[36mController Detected! '%s'\033[0m\n", GetGamepadName(gst->gamepad.id));
    }

    gst->shadow_bias = 2.0;

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

