

#include <raylib.h>
#include <stdio.h>
#include <math.h>
#include <raymath.h>
#include <time.h>
#include <string.h>
#include <rlgl.h>

#include "state.h"
#include "input.h"
#include "util.h"
#include "terrain.h"

#define SCRN_W 1200
#define SCRN_H 800

#define GLSL_VERSION 330


void cleanup(struct state_t* gst);
RenderTexture2D LoadRenderTextureDepthTex(int width, int height);
void UnloadRenderTextureDepthTex(RenderTexture2D target);

void loop(struct state_t* gst) {

    Model testmodel = LoadModelFromMesh(GenMeshKnot(1.3, 3.5, 64, 64));
    testmodel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = gst->textures[GRID4x4_TEXID];
    testmodel.materials[0].shader = gst->shaders[DEFAULT_SHADER];



    gst->scrn_w = GetScreenWidth();
    gst->scrn_h = GetScreenHeight();

    gst->env_render_target = LoadRenderTexture(gst->scrn_w, gst->scrn_h);
    gst->bloomtreshold_target = LoadRenderTexture(gst->scrn_w, gst->scrn_h);


    while(!WindowShouldClose()) {

        gst->dt = GetFrameTime();
        gst->time = GetTime();



        {
            const int sw = GetScreenWidth();
            const int sh = GetScreenHeight();
            if((sw != gst->scrn_w) || (sh != gst->scrn_h)) {
                gst->scrn_w = sw;
                gst->scrn_h = sh;
                UnloadRenderTexture(gst->env_render_target);
                UnloadRenderTexture(gst->bloomtreshold_target);
                gst->env_render_target = LoadRenderTexture(gst->scrn_w, gst->scrn_h);
                gst->bloomtreshold_target = LoadRenderTexture(gst->scrn_w, gst->scrn_h);
              
                printf(" Resized to %ix%i\n", sw, sh);
            }
        }

      
        state_update_frame(gst);
        state_update_shader_uniforms(gst);
        state_render_environment(gst);



        BeginDrawing();
        {
            ClearBackground(BLACK);


            // Finally post process everything.
            //
            BeginShaderMode(gst->shaders[POSTPROCESS_SHADER]);
            {
                rlEnableShader(gst->shaders[POSTPROCESS_SHADER].id);
                rlSetUniformSampler(GetShaderLocation(gst->shaders[POSTPROCESS_SHADER],
                            "bloom_treshold_texture"), gst->bloomtreshold_target.texture.id);
                
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



            if(IsKeyPressed(KEY_F)) {
                gst->player.health -= GetRandomValue(1, 50);
            }

            // Draw player stats.
            {
                const int bar_width = 150;
                const int bar_height = 10;
            
                int bar_next_y = 10;
                int bar_inc = 15;

                // Health
                {

                    DrawRectangle(
                            10,
                            bar_next_y,
                            bar_width,
                            bar_height + 5,
                            (Color){ 20, 20, 20, 255 }
                            );



                    DrawRectangle(
                            10,
                            bar_next_y,
                            map(gst->player.health, 0.0, gst->player.max_health, 0, bar_width),
                            bar_height + 5,
                            color_lerp(
                                normalize(gst->player.health, 0.0, gst->player.max_health),
                                PLAYER_HEALTH_COLOR_LOW,
                                PLAYER_HEALTH_COLOR_HIGH
                                )
                            );


                }
                bar_next_y += bar_inc+5;

                // Fire rate timer.
                {
                    const float timervalue = gst->player.firerate_timer;

                    DrawRectangle(
                            10,
                            bar_next_y,
                            bar_width,
                            bar_height,
                            (Color){ 20, 20, 20, 255 }
                            );

                    DrawRectangle(
                            10,
                            bar_next_y,
                            map(timervalue, 0.0, gst->player.firerate, 0, bar_width),
                            bar_height,
                            (Color){ 10, 180, 255, 255 }
                            );
                }
                bar_next_y += bar_inc;

                // Weapon temperature.
                {
                    const float tempvalue = gst->player.weapon.temp;

                    DrawRectangle(
                            10,
                            bar_next_y,
                            bar_width,
                            bar_height,
                            (Color){ 20, 20, 20, 255 }
                            );

                    DrawRectangle(
                            10,
                            bar_next_y,
                            map(tempvalue, 0.0, gst->player.weapon.overheat_temp, 0, bar_width),
                            bar_height,
                            (Color){ 255, 58, 30, 255 }
                            );
                }
                bar_next_y += bar_inc;

                // Weapon Accuracy
                {
                    const float accvalue 
                        = gst->player.weapon.accuracy - gst->player.accuracy_decrease;
                    DrawRectangle(
                            10,
                            bar_next_y,
                            bar_width,
                            bar_height,
                            (Color){ 20, 20, 20, 255 }
                            );

                    DrawRectangle(
                            10,
                            bar_next_y,
                            map(accvalue, WEAPON_ACCURACY_MIN, WEAPON_ACCURACY_MAX, 0, bar_width),
                            bar_height,
                            (Color){ 255, 200, 30, 255 }
                            );
                }
            }


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


            // Some info for player:


            // TODO: do this with texture...
            if(gst->player.weapon.temp >= (gst->player.weapon.overheat_temp*0.7)) {
                DrawText("(WARNING: OVERHEATING...)", 29.0, gst->scrn_h-149, 20,
                        (Color){ 50, 30, 30, 255 });
                DrawText("(WARNING: OVERHEATING...)", 30.0, gst->scrn_h-150, 20,
                        (Color){ (sin(gst->time*10.0)*0.5+0.5)*125+120, 30, 30, 255 });    
            }


            {
                const char* weapon_text = TextFormat("(x) [%s]", (gst->player.weapon_firetype) ? "SemiAuto" : "FullAuto");
                DrawText(weapon_text,
                        29.0, gst->scrn_h-99, 20, (Color){20, 40, 40, 255});
 
                DrawText(weapon_text,
                        30.0, gst->scrn_h-100, 20, (Color){ 30, 100, 100, 255 });
            }
           





            DrawText(TextFormat("x: %0.2f", gst->player.position.x),
                        15.0, gst->scrn_h-30, 20, BLACK);
                
            DrawText(TextFormat("y: %0.2f", gst->player.position.x),
                        15.0+100.0, gst->scrn_h-30, 20, BLACK);
                
            DrawText(TextFormat("z: %0.2f", gst->player.position.x),
                        15.0+100.0*2, gst->scrn_h-30.0, 20, BLACK);
 


            DrawText(TextFormat("FPS: %i", GetFPS()),
                    gst->scrn_w - 100, gst->scrn_h-30, 20, WHITE);

            if(gst->debug) {
                DrawText("(Debug ON)", gst->scrn_w - 200, 10, 20, RED);
            }
        }
        EndDrawing();
    }


    UnloadModel(testmodel);
}



void cleanup(struct state_t* gst) {
    
    // Unload all textures
    for(unsigned int i = 0; i < gst->num_textures; i++) {
        UnloadTexture(gst->textures[i]);
    }

    // Unload all entity models
    for(size_t i = 0; i < gst->num_entities; i++) {
        delete_entity(&gst->entities[i]);
    }

    for(size_t i = 0; i < gst->num_entity_weapons; i++) {
        delete_weapon(&gst->entity_weapons[i]);
    }

    
    UnloadShader(gst->shaders[DEFAULT_SHADER]);
    UnloadShader(gst->shaders[POSTPROCESS_SHADER]);
    UnloadShader(gst->shaders[PROJECTILES_PSYSTEM_SHADER]); 
    UnloadShader(gst->shaders[BLOOM_TRESHOLD_SHADER]); 
    free_player(&gst->player);

    delete_psystem(&gst->psystems[PROJECTILE_ENVHIT_PSYSTEM]);
    delete_psystem(&gst->psystems[PROJECTILE_ENTITYHIT_PSYSTEM]);
    

    delete_terrain(&gst->terrain);

    UnloadRenderTexture(gst->env_render_target);
    UnloadRenderTexture(gst->bloomtreshold_target);

    CloseWindow();
}


void load_texture(struct state_t* gst, const char* filepath, int texid) {
    
    if(texid >= MAX_TEXTURES) {
        fprintf(stderr, "ERROR: Texture id is invalid. (%i) for '%s'\n", texid, filepath);
        return;
    }
    gst->textures[texid] = LoadTexture(filepath);
    gst->num_textures++;

    printf("\033[32m + Loaded (texid:%i) '%s'\033[0m \n", texid, filepath);
}

void first_setup(struct state_t* gst) {

    InitWindow(SCRN_W, SCRN_H, "Game");

    DisableCursor();
    SetTargetFPS(500);
    gst->num_textures = 0;
    gst->debug = 0;
    gst->num_entities = 0;
    gst->num_normal_lights = 0;
    gst->num_projectile_lights = 0;
    gst->dt = 0.016;
    gst->num_entity_weapons = 0;

    memset(gst->fs_unilocs, 0, MAX_FS_UNILOCS * sizeof *gst->fs_unilocs);
    memset(gst->entities, 0, MAX_ENTITIES * sizeof *gst->entities);

    const float fog_density = 0.006;
    const float terrain_scale = 8.0;
    const u32   terrain_size = 500;
    const float terrain_amplitude = 30.0;
    const float terrain_pnfrequency = 10.0;
    const int   terrain_octaves = 3;
    const Vector3 sun_position = (Vector3) { 0.0, 0.5, -0.9 };


    // --- Load textures ---
    
    load_texture(gst, "res/textures/grid_4x4.png", GRID4x4_TEXID);
    load_texture(gst, "res/textures/grid_6x6.png", GRID6x6_TEXID);
    load_texture(gst, "res/textures/grid_9x9.png", GRID9x9_TEXID);
    load_texture(gst, "res/textures/gun_0.png", GUN_0_TEXID);
    load_texture(gst, "res/textures/enemy_lvl0.png", ENEMY_LVL0_TEXID);
    load_texture(gst, "res/textures/arms.png", PLAYER_ARMS_TEXID);

    // --- Setup Default Shader ---
    {
  
        Shader* shader = &gst->shaders[DEFAULT_SHADER];

        *shader = LoadShader(
            "res/shaders/default.vs",
            "res/shaders/default.fs"
        );

        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(*shader, "matModel");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
   
        int ambientloc = GetShaderLocation(*shader, "ambient");
        int fogdensityloc = GetShaderLocation(*shader, "fogDensity");
        
        SetShaderValue(*shader, ambientloc, (float[4]){ 0.5, 0.5, 0.5, 1.0}, SHADER_UNIFORM_VEC4);
        SetShaderValue(*shader, fogdensityloc, &fog_density, SHADER_UNIFORM_FLOAT);
    }


    // --- Setup Post Processing Shader ---
    {
        Shader* shader = &gst->shaders[POSTPROCESS_SHADER];
        *shader = LoadShader(
            0 /* use raylibs default vertex shader */, 
            "res/shaders/postprocess.fs"
        );

        gst->fs_unilocs[POSTPROCESS_TIME_FS_UNILOC] = GetShaderLocation(*shader, "time");
        gst->fs_unilocs[POSTPROCESS_SCREENSIZE_FS_UNILOC] = GetShaderLocation(*shader, "screen_size");
        gst->fs_unilocs[POSTPROCESS_PLAYER_HEALTH_FS_UNILOC] = GetShaderLocation(*shader, "health");
    }

 
    // --- Setup Projectiles (Particle System) shader ---
    {
        Shader* shader = &gst->shaders[PROJECTILES_PSYSTEM_SHADER];
        *shader = LoadShader(
            "res/shaders/particle_core.vs",
            "res/shaders/projectiles_psystem.fs"
        );
       
        shader->locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(*shader, "mvp");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
        
        gst->fs_unilocs[PROJECTILES_PSYSTEM_COLOR_FS_UNILOC] = GetShaderLocation(*shader, "psystem_color");
       
    }



    // --- Setup Bloom Treshold Shader ---
    {
        Shader* shader = &gst->shaders[BLOOM_TRESHOLD_SHADER];
        *shader = LoadShader(
            0 /* use raylibs default vertex shader */, 
            "res/shaders/bloom_treshold.fs"
        );
    }




    init_player_struct(gst, &gst->player);



    // --- Setup Terrain ----
    {
        init_perlin_noise();
        gst->terrain = (struct terrain_t) { 0 };

        generate_terrain(
                gst, &gst->terrain,
                terrain_size,
                terrain_scale,
                terrain_amplitude,
                terrain_pnfrequency,
                terrain_octaves
                );
    }

    int seed = time(0);
    gst->rseed = seed;
    SetRandomSeed(seed);


    // --- Create Weapons for entities ---
    {

        // Weapon for ENEMY_LVL0
        //
        setup_weapon(gst,
                &gst->entity_weapons[ENTWEAPON_LVL0],
                enemy_lvl0_weapon_psystem_projectile_pupdate,
                enemy_lvl0_weapon_psystem_projectile_pinit,
                (struct weapon_t)
                {
                    .accuracy = 6.75,
                    .prj_speed = 100,
                    .prj_damage = 10,
                    .prj_max_lifetime = 3.0,
                    .prj_size = (Vector3){ 1.0, 1.0, 1.0 },
                    .prj_color = (Color) { 255, 50, 255,  255 }
                }
        );
        gst->num_entity_weapons++; // <- NOTE: keep track of this!


    }


    // --- Create Particle System  (PROJECTILE_ENVHIT_PSYSTEM) ---

    {
        struct psystem_t* psystem = &gst->psystems[PROJECTILE_ENVHIT_PSYSTEM];
        create_psystem(gst,
                psystem, 32,
                projectile_envhit_psystem_pupdate,
                projectile_envhit_psystem_pinit
                );

        psystem->particle_material = LoadMaterialDefault();
        psystem->particle_material.shader = gst->shaders[PROJECTILES_PSYSTEM_SHADER];
        psystem->particle_mesh = GenMeshSphere(0.3, 16, 16);

    }


    // --- Create Particle System  (PROJECTILE_ENTITYHIT_PSYSTEM) ---

    {
        struct psystem_t* psystem = &gst->psystems[PROJECTILE_ENTITYHIT_PSYSTEM];
        create_psystem(gst,
                psystem, 512,
                projectile_entityhit_psystem_pupdate,
                projectile_entityhit_psystem_pinit
                );

        psystem->particle_material = LoadMaterialDefault();
        psystem->particle_material.shader = gst->shaders[PROJECTILES_PSYSTEM_SHADER];
        psystem->particle_mesh = GenMeshSphere(0.3, 8, 8);

    }




    // --- Create entities (FOR TESTING) ---

    for(int i = 0; i < 20; i++) {

        Vector3 pos = (Vector3){ 0 };

        while(Vector3Distance(pos, gst->player.position) < 50) {
            pos = (Vector3){ RSEEDRANDOMF(-400, 400), 0.0, RSEEDRANDOMF(-400, 400) };
        }

        create_entity(gst,
                ENT_TYPE_LVL0,
                ENT_TRAVELING_DISABLED,
                "res/models/lvl0_enemy.glb",
                ENEMY_LVL0_TEXID,
                100,
                pos, // initial position
                (Vector3){ 3.0, 3.0, 3.0 }, // hitbox size
                (Vector3){ 0.0, 1.5, 0.0 }, // hitbox position
                130.0,  // target range
                0.3     // firerate
                );

    }


    // --- Add sun ---
   
    add_light(gst,
            LIGHT_DIRECTIONAL,
            sun_position,
            (Color) { 220, 170, 230, 255 },
            gst->shaders[DEFAULT_SHADER]
            );
   
    add_light(gst,
            LIGHT_POINT,
            (Vector3){-1, 3, 0 },
            (Color) { 200, 20, 50, 255 },
            gst->shaders[DEFAULT_SHADER]
            );
   


}

RenderTexture2D LoadRenderTextureDepthTex(int width, int height) {
    RenderTexture2D target = { 0 };
    target.id = rlLoadFramebuffer();

    if(target.id > 0) {
        rlEnableFramebuffer(target.id);

         // Create color texture (default to RGBA)
        target.texture.id = rlLoadTexture(0, width, height, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
        target.texture.width = width;
        target.texture.height = height;
        target.texture.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        target.texture.mipmaps = 1;

         // Create depth texture buffer (instead of raylib default renderbuffer)
         target.depth.id = rlLoadTextureDepth(width, height, false);
         target.depth.width = width;
         target.depth.height = height;
         target.depth.format = 19;       //DEPTH_COMPONENT_24BIT?
         target.depth.mipmaps = 1;

         // Attach color texture and depth texture to FBO
         rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0,
                 RL_ATTACHMENT_TEXTURE2D, 0);
         rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D,0);

         // Check if fbo is complete with attachments (valid)

         rlDisableFramebuffer();
     }
     else TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");

     return target;

}


int main(void) {

    struct state_t gst;

    first_setup(&gst);
    loop(&gst);
    cleanup(&gst);


    printf("Exit 0.\n");
    return 0;
}
