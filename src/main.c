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

    Model testmodel = LoadModelFromMesh(GenMeshKnot(1.3, 3.5, 64, 64));
    testmodel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = gst->textures[GRID4x4_TEXID];
    testmodel.materials[0].shader = gst->shaders[DEFAULT_SHADER];


    //gst->depth_texture = LoadShadowmapRenderTexture(1024, 1024);

    while(!WindowShouldClose() && gst->running) {

        gst->dt = GetFrameTime();
        gst->time = GetTime();



        {
            const int sw = GetScreenWidth();
            const int sh = GetScreenHeight();
            if((sw != gst->scrn_w) || (sh != gst->scrn_h)) {
                gst->scrn_w = sw;
                gst->scrn_h = sh;
                UnloadRenderTexture(gst->env_render_target);
                UnloadRenderTexture(gst->bloomtresh_target);
                gst->env_render_target = LoadRenderTexture(gst->scrn_w, gst->scrn_h);
                gst->bloomtresh_target = LoadRenderTexture(gst->scrn_w, gst->scrn_h);
              
                printf(" Resized to %ix%i\n", sw, sh);
            }
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

                // TODO: Optimize this.
                SetShaderValueTexture(gst->shaders[POSTPROCESS_SHADER],
                        GetShaderLocation(gst->shaders[POSTPROCESS_SHADER],
                            "bloomtresh_texture"), gst->bloomtresh_target.texture);

                SetShaderValueTexture(gst->shaders[POSTPROCESS_SHADER],
                        GetShaderLocation(gst->shaders[POSTPROCESS_SHADER],
                            "depth_texture"), gst->depth_texture.texture);
    

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
            
            // Some info for player:

                /*
                // Experience level
                DrawText(TextFormat("XP: %i", gst->player.xp), 50, gst->scrn_h-50, 
                        30, (Color){ 30, 255, 30, 255 });
                        */
            
            if(gst->player.item_in_crosshair && !gst->player.inventory.open) {
                struct item_t* item = gst->player.item_in_crosshair;

                Vector2 item_name_pos = (Vector2) {
                    gst->scrn_w/2 - item->name_width / 2 - 200,
                    gst->scrn_h/2 + 100,
                };

                DrawText(item->name, item_name_pos.x, item_name_pos.y, 20, WHITE);
                DrawText("< Press (F) to pickup >\0", 
                        item_name_pos.x, item_name_pos.y+30, 20,
                        (Color){ 100, 100, 100, 255 });

                if(IsKeyPressed(KEY_F)) {
                    item->enabled = !inv_add_item(gst, &gst->player, item);
                }
                
            }




            /*
            {
                const char* weapon_text = TextFormat("(x) [%s]", (gst->player.weapon_firetype) ? "SemiAuto" : "FullAuto");
                DrawText(weapon_text,
                        29.0, gst->scrn_h-99, 20, (Color){20, 40, 40, 255});
 
                DrawText(weapon_text,
                        30.0, gst->scrn_h-100, 20, (Color){ 30, 100, 100, 255 });
            } 
            */



            if(gst->debug) {
                int next_y = 300;
                const int y_inc = 25;

                DrawText(TextFormat("X: %0.2f", gst->player.position.x),
                            15.0, next_y, 20, GREEN);
                DrawText(TextFormat("Y: %0.2f", gst->player.position.y),
                            15.0+130.0, next_y, 20, GREEN);
                DrawText(TextFormat("Z: %0.2f", gst->player.position.z),
                            15.0+130.0*2, next_y, 20, GREEN);
   
                next_y += y_inc;
                DrawText(TextFormat("NumVisibleChunks: %i", gst->terrain.num_visible_chunks),
                        15.0, next_y, 20, PURPLE);
                
                DrawText("(Debug ON)", gst->scrn_w - 200, 10, 20, GREEN);
            }


            DrawText(TextFormat("FPS: %i", GetFPS()),
                    gst->scrn_w - 100, gst->scrn_h-30, 20, WHITE);


        }
        EndDrawing();
    }


    UnloadModel(testmodel);
}



void cleanup(struct state_t* gst) {
    
   
    delete_terrain(&gst->terrain);
    delete_player(&gst->player);

    state_delete_all_textures(gst);
    state_delete_all_shaders(gst);
    state_delete_all_psystems(gst);
    state_delete_all_sounds(gst);
    state_delete_all_enemy_models(gst);
    state_delete_all_item_models(gst);

    UnloadRenderTexture(gst->env_render_target);
    UnloadRenderTexture(gst->bloomtresh_target);
    UnloadRenderTexture(gst->depth_texture);

    for(int i = 0; i < MAX_UBOS; i++) {
        glDeleteBuffers(1, &gst->ubo[i]);
    }

    UnloadFont(gst->font);

    printf("\033[35m -> Cleanup done...\033[0m\n");
    CloseWindow();
}



void first_setup(struct state_t* gst) {

    InitWindow(DEF_SCRN_W, DEF_SCRN_H, "331uw13's 3D-Game");
    SetExitKey(-1);
    gst->font = LoadFont("res/Topaz-8.ttf");

    
    DisableCursor();
    SetTargetFPS(TARGET_FPS);
    SetTraceLogLevel(LOG_ERROR);
    rlSetClipPlanes(rlGetCullDistanceNear()+0.15, rlGetCullDistanceFar()+3000);

    gst->num_textures = 0;
    gst->num_textures = 0;
    gst->debug = 0;
    gst->num_enemies = 0;
    gst->num_prj_lights = 0;
    gst->dt = 0.016;
    gst->num_enemy_weapons = 0;
    gst->menu_open = 0;
    gst->running = 1;

    memset(gst->fs_unilocs, 0, MAX_FS_UNILOCS * sizeof *gst->fs_unilocs);
    memset(gst->enemies, 0, MAX_ALL_ENEMIES * sizeof *gst->enemies);


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

    state_setup_all_psystems(gst);
    
    setup_natural_item_spawn_settings(gst);
    setup_default_enemy_spawn_settings(gst);

    gst->fog_density = 2.0;
    gst->fog_color_near = (Color){ 50, 170, 200 };
    gst->fog_color_far = (Color){ 80, 50, 70 };

    update_fog_settings(gst);
    
    set_powerup_defaults(gst, &gst->player.powerup_shop);
    update_powerup_shop_offers(gst);


    init_player_struct(gst, &gst->player);

    state_setup_render_targets(gst);

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


    struct light_t SUN = (struct light_t) {
        .type = LIGHT_DIRECTIONAL,
        .enabled = 1,
        .color = (Color){ 240, 210, 200, 255 },
        .position = (Vector3){ -1, 1, -1 },
        .strength = 1.0,
        .index = SUN_LIGHT_ID
    };

    gst->player.gun_light = (struct light_t) {
        .type = LIGHT_POINT,
        .enabled = 1,
        .strength = 0.35,
        .radius = 2.0,
        .index = PLAYER_GUN_LIGHT_ID
    }; // Player's gun light is updated from 'src/player.c'


    set_light(gst, &SUN, LIGHTS_UBO);
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

