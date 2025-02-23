

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


void loop(struct state_t* gst) {

   
    /*
    Model testfloor = LoadModelFromMesh(GenMeshCube(40.0, 0.25, 40.0));
    testfloor.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = gst->tex[GRID9x9_TEXID];
    testfloor.materials[0].shader = gst->shaders[DEFAULT_SHADER];
    */

    //create_object(gst, "res/models/street-lamp.glb", NONE_TEXID, (Vector3){ -3.0, 0.0, 0.0 });

    Model testmodel = LoadModelFromMesh(GenMeshKnot(1.0, 3.0, 32, 32));
    testmodel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = gst->textures[GRID4x4_TEXID];
    testmodel.materials[0].shader = gst->shaders[DEFAULT_SHADER];



    int scrn_w = GetScreenWidth();
    int scrn_h = GetScreenHeight();

    RenderTexture2D render_target = LoadRenderTexture(scrn_w, scrn_h);



    while(!WindowShouldClose()) {
        float dt = GetFrameTime();

        gst->dt = dt;
        gst->time = GetTime();



        {
            const int sw = GetScreenWidth();
            const int sh = GetScreenHeight();

            if((sw != scrn_w) || (sh != scrn_h)) {
                scrn_w = sw;
                scrn_h = sh;
                UnloadRenderTexture(render_target);
                render_target = LoadRenderTexture(scrn_w, scrn_h);
               
                printf(" Resized to %ix%i\n", sw, sh);
            }

        }

       

        // Update view position for shaders.

        float camposf3[3] = { gst->player.cam.position.x, gst->player.cam.position.y, gst->player.cam.position.z };
        
        SetShaderValue(gst->shaders[DEFAULT_SHADER], 
                gst->shaders[DEFAULT_SHADER].locs[SHADER_LOC_VECTOR_VIEW], camposf3, SHADER_UNIFORM_VEC3);

                
        SetShaderValue(gst->shaders[PROJECTILES_PSYSTEM_SHADER], 
                gst->shaders[PROJECTILES_PSYSTEM_SHADER].locs[SHADER_LOC_VECTOR_VIEW], camposf3, SHADER_UNIFORM_VEC3);



        // Update time for shaders.

        SetShaderValue(
                gst->shaders[POSTPROCESS_SHADER],
                gst->fs_unilocs[POSTPROCESS_TIME_FS_UNILOC],
                &gst->time,
                SHADER_UNIFORM_FLOAT
                );

        SetShaderValue(
                gst->shaders[POSTPROCESS_SHADER],
                gst->fs_unilocs[POSTPROCESS_PLAYER_HEALTH_FS_UNILOC],
                &gst->player.health_normalized,
                SHADER_UNIFORM_FLOAT
                );

        // Update frame buffer size for post process shader

        {

            const float screen_size[2] = {
                GetScreenWidth(), GetScreenHeight()
            };
            SetShaderValue(
                    gst->shaders[POSTPROCESS_SHADER],
                    gst->fs_unilocs[POSTPROCESS_SCREENSIZE_FS_UNILOC],
                    screen_size,
                    SHADER_UNIFORM_VEC2
                    );

        }


        // --- Rendering ---

        // Render 3D stuff into texture and post process it later.
        BeginTextureMode(render_target);
        ClearBackground((Color){ 
                (0.3) * 255,
                (0.15) * 255,
                (0.15) * 255,
                255 });

        BeginMode3D(gst->player.cam);
        {
            BeginShaderMode(gst->shaders[DEFAULT_SHADER]);


            // Test thing.
            {
                testmodel.transform = MatrixRotateXYZ((Vector3){ gst->time, 0.0, gst->time });
                DrawModel(testmodel, (Vector3){ 5.0, 5.0, -5.0 }, 1.0, (Color){ 255, 255, 255, 255});

            }

            // Terrain.
            render_terrain(gst, &gst->terrain);

            // Entities.
            for(size_t i = 0; i < gst->num_entities; i++) {
                update_entity(gst, &gst->entities[i], ENT_RENDER_ON_UPDATE);
            }

            // Player.
            player_render(gst, &gst->player);

            // Weapons.
            weapon_update(gst, &gst->player.weapon);

            for(size_t i = 0; i < gst->num_entity_weapons; i++) {
                weapon_update(gst, &gst->entity_weapons[i]);
            }


            EndShaderMode();
        }

        EndMode3D();
        EndTextureMode();



        BeginDrawing();
        {
            ClearBackground(BLACK);


            BeginShaderMode(gst->shaders[POSTPROCESS_SHADER]);
            {
                DrawTextureRec(
                        render_target.texture,
                        (Rectangle) { 
                            0.0, 0.0, 
                            (float)render_target.texture.width,
                            -(float)render_target.texture.height
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

                int bar_inc = 20;
                int bar_next_y = 10;

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
                bar_next_y += bar_inc;

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
            }


            int center_x = GetScreenWidth() / 2;
            int center_y = GetScreenHeight() / 2;
            DrawPixel(center_x, center_y, WHITE);
            

            // text and info.
                /*
            {
                // draw cursor.



                DrawText(TextFormat("FPS(%i)", GetFPS()),
                        15.0, 40.0, 20.0, (Color){ 150, 150, 150, 255 });

                DrawText(TextFormat("NoClip(%s)", gst->player.noclip ? "ON" : "OFF"),
                        120.0, 40.0, 20.0, (Color){ 150, 150, 150, 255 });


                DrawText(TextFormat("x: %0.2f", gst->player.position.x),
                        15.0, 10.0, 20.0, (Color){ 200, 100, 20, 255});
                
                DrawText(TextFormat("y: %0.2f", gst->player.position.x),
                        15.0+100.0, 10.0, 20.0, (Color){ 100, 200, 20, 255});
                
                DrawText(TextFormat("z: %0.2f", gst->player.position.x),
                        15.0+100.0*2, 10.0, 20.0, (Color){ 20, 100, 200, 255});
            

                DrawText(TextFormat("Accuracy:            %0.3f", gst->player.weapon.accuracy),
                        15.0, 100.0, 20.0, RED);
                DrawText(TextFormat("Accuracy (Mapped):   %0.3f", compute_weapon_accuracy(gst, &gst->player.weapon)),
                        15.0, 130.0, 20.0, RED);


                if(IsKeyDown(KEY_ONE)) {
                    gst->player.weapon.accuracy -= 0.01;
                    if(gst->player.weapon.accuracy < 0) {
                        gst->player.weapon.accuracy = 0.0;
                    }
                }
                else if(IsKeyDown(KEY_TWO)) {
                    gst->player.weapon.accuracy += 0.01;
                    if(gst->player.weapon.accuracy > 10.0) {
                        gst->player.weapon.accuracy = 10.0;
                    }
                }
            }
            */

        }
        EndDrawing();

        handle_userinput(gst);
        player_update(gst, &gst->player);
    }


    UnloadModel(testmodel);
    UnloadRenderTexture(render_target);
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

    
    //free_objarray(gst);
    //delete_psystem(&gst->psystems[PSYS_ENEMYHIT]);
    //free_enemyarray(gst);
    free_player(&gst->player);
    UnloadShader(gst->shaders[DEFAULT_SHADER]);
    UnloadShader(gst->shaders[POSTPROCESS_SHADER]);
    UnloadShader(gst->shaders[PROJECTILES_PSYSTEM_SHADER]); 
    delete_terrain(&gst->terrain);



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
    gst->draw_debug = 0;
    gst->objects = NULL;
    gst->objarray_size = 0;
    gst->num_objects = 0;
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
    // -----------------------


    /*
    // --- Setup (ENEMY_HIT) Particle System. ---
    {
        create_psystem(
                gst,
                &gst->psystems[PSYS_ENEMYHIT],
                1024,
                enemy_hit_psys_pupdate,
                enemy_hit_psys_pinit
                );
        struct psystem_t* psys = &gst->psystems[PSYS_ENEMYHIT];

        /psys->particle_material = LoadMaterialDefault();
        psys->particle_material.shader = gst->shaders[ENEMY_HIT_PSYS_SHADER];
        psys->particle_mesh = GenMeshCube(0.2, 0.2, 0.2);
    }
    */

    /* TODO!!!!!!
    // Make sure all lights are disabled.
    for(int i = 0; i < MAX_LIGHTS; i++) {
        int enabled = 0;
        int loc = GetShaderLocation(gst->shaders[DEFAULT_SHADER], TextFormat("lights[%i].enabled", i));
        SetShaderValue(gst->shaders[DEFAULT_SHADER], loc, &enabled, SHADER_UNIFORM_INT);
    }
    */

    /*
    // -------------- TODO: move this into player.c -------------
    // Make sure all projectiles are intialized correctly.
    struct weapon_t* player_weapon = &gst->player.weapon;
    for(int i = 0; i < MAX_WEAPON_PROJECTILES; i++) {
        player_weapon->projectiles[i].position = (Vector3){ 0 };
        player_weapon->projectiles[i].direction = (Vector3){ 0 };
        player_weapon->projectiles[i].lifetime = 0.0;
        player_weapon->projectiles[i].alive = 0;
    }
    */


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
                    .accuracy = 8.75,
                    .prj_speed = 90,
                    .prj_damage = 10,
                    .prj_max_lifetime = 3.0,
                    .prj_size = (Vector3){ 1.0, 1.0, 1.0 },
                    .prj_color = (Color) { 255, 50, 20,  255 }
                }
        );
        gst->num_entity_weapons++; // <- NOTE: keep track of this!


    }


    // --- Create entities (FOR TESTING) ---

    for(int i = 0; i < 20; i++) {

        Vector3 pos = (Vector3){ 0 };

        while(Vector3Distance(pos, gst->player.position) < 100) {
            pos = (Vector3){ RSEEDRANDOMF(-400, 400), 0.0, RSEEDRANDOMF(-400, 400) };
        }

        create_entity(gst,
                ENT_TYPE_LVL0,
                ENT_TRAVELING_DISABLED,
                "res/models/lvl0_enemy.glb",
                ENEMY_LVL0_TEXID,
                100,
                pos, // initial position
                (Vector3){ 1.0, 1.0, 1.0 },    // hitbox size
                100.0,  // target range
                0.3     // firerate
                );

    }


    // --- Add sun ---
   
    add_light(gst,
            LIGHT_DIRECTIONAL,
            sun_position,
            (Color) { 200, 100, 30, 255 },
            gst->shaders[DEFAULT_SHADER]
            );
   
    add_light(gst,
            LIGHT_POINT,
            (Vector3){-5, 3, 0 },
            (Color) { 2, 200, 30, 255 },
            gst->shaders[DEFAULT_SHADER]
            );
   


}


int main(void) {

    struct state_t gst;

    first_setup(&gst);
    loop(&gst);
    cleanup(&gst);


    printf("Exit 0.\n");
    return 0;
}
