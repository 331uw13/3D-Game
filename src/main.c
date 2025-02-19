

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
#define SCRN_H 700

#define GLSL_VERSION 330


void cleanup(struct state_t* gst);


void loop(struct state_t* gst) {

   
    /*
    Model testfloor = LoadModelFromMesh(GenMeshCube(40.0, 0.25, 40.0));
    testfloor.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = gst->tex[GRID9x9_TEXID];
    testfloor.materials[0].shader = gst->shaders[DEFAULT_SHADER];
    */

    create_object(gst, "res/models/street-lamp.glb", NONE_TEXID, (Vector3){ -3.0, 0.0, 0.0 });



    /*
   
    create_enemy(gst,
            "res/models/enemy.glb", ENEMY_0_TEXID,
            ENEMY_0_MAX_HEALTH,
            (Vector3) { 2.0, 2.0, 2.0 }, // hitbox
            (Vector3) { -12.0, 2.0, 3.0 } // position

            );

    create_enemy(gst,
            "res/models/enemy.glb", ENEMY_0_TEXID,
            ENEMY_0_MAX_HEALTH,
            (Vector3) { 2.0, 2.0, 2.0 }, // hitbox
            (Vector3) { 12.0, 2.0, 8.0 } // position 

            );

    create_enemy(gst,
            "res/models/enemy.glb", ENEMY_0_TEXID,
            ENEMY_0_MAX_HEALTH,
            (Vector3) { 2.0, 2.0, 2.0 }, // hitbox
            (Vector3) { 12.0, 0.0, -7.0 } // position

            );

            */

    /*
    for(int i = 0; i < 10; i++) {
    
        create_enemy(gst,
            "res/models/enemy.glb", ENEMY_0_TEXID,
            ENEMY_0_MAX_HEALTH,
            
            // hitbox
            (Vector3) { 2.0, 2.0, 2.0 }, 
            
            // position
            (Vector3) { GetRandomValue(-80,80), 0.0, GetRandomValue(-80,80) }

            );


    }
    */

    /*
    init_perlin_noise();

    struct terrain_t terrain = { 0 };
    generate_heightmap(&terrain);
    generate_terrain_mesh(gst, &terrain);

    */
    while(!WindowShouldClose()) {
        float dt = GetFrameTime();
        float time = GetTime();

        gst->dt = dt;

        // --- update movement. ---

       

        // update view position for shaders.

        float camposf3[3] = { gst->player.cam.position.x, gst->player.cam.position.y, gst->player.cam.position.z };
        
        SetShaderValue(gst->shaders[DEFAULT_SHADER], 
                gst->shaders[DEFAULT_SHADER].locs[SHADER_LOC_VECTOR_VIEW], camposf3, SHADER_UNIFORM_VEC3);

                
        SetShaderValue(gst->shaders[PLANET_SKYBOX_SHADER],
                gst->fs_unilocs[PLANET_SKYBOX_VIEWPOS_FS_UNILOC], camposf3, SHADER_UNIFORM_VEC3);
       
        SetShaderValue(gst->shaders[PLANET_SKYBOX_SHADER],
                gst->fs_unilocs[PLANET_SKYBOX_GLOBALTIME_FS_UNILOC], &time, SHADER_UNIFORM_FLOAT);
       
        /*
        SetShaderValue(gst->shaders[TEST_PSYS_SHADER], 
                gst->shaders[TEST_PSYS_SHADER].locs[SHADER_LOC_VECTOR_VIEW], camposf3, SHADER_UNIFORM_VEC3);
        */



        float sunpos[4] = { 
            0.0,
            sin(0.5),
            cos(0.5)
        };


        SetShaderValue(
                gst->shaders[PLANET_SKYBOX_SHADER]
                , gst->fs_unilocs[PLANET_SUN_POSITION_FS_UNILOC], sunpos, SHADER_UNIFORM_VEC3);
       
        gst->lights[0].position = Vector3Scale((Vector3){sunpos[0], sunpos[1], sunpos[2]},
                gst->terrain.heightmap.size);

        UpdateLightValues(gst->shaders[DEFAULT_SHADER], gst->lights[0]);


        // --- render. ---

        BeginDrawing();
        {
            ClearBackground((Color){ 10, 10, 10, 255 });

    

            // Draw 3D stuff
            BeginMode3D(gst->player.cam);
            {
               
                rlDisableBackfaceCulling();
                rlDisableDepthMask();


                gst->skybox_transform = MatrixTranslate(
                        gst->player.cam.position.x,
                        gst->player.cam.position.y,
                        gst->player.cam.position.z);
                DrawMesh(gst->skybox_mesh, gst->skybox_material, gst->skybox_transform);

                rlEnableBackfaceCulling();
                rlEnableDepthMask();
           


                BeginShaderMode(gst->shaders[DEFAULT_SHADER]);

                // Draw objects
                //
                for(size_t i = 0; i < gst->num_objects; i++) {
                    struct obj_t* obj = &gst->objects[i];
                    DrawMesh(
                            obj->model.meshes[0],
                            obj->model.materials[0],
                            obj->model.transform
                            );
                
                }

                /*
                // Draw enemies.
                //
                for(size_t i = 0; i < gst->num_enemies; i++) {
                    struct enemy_t* enemy = &gst->enemies[i];
                    update_enemy(gst, enemy);
                    render_enemy(gst, enemy);
                }
                */



                render_terrain(gst, &gst->terrain);
                player_render(gst, &gst->player);


                EndShaderMode(); // -----------



            }
            EndMode3D();


            // Draw 2D stuff
            {
                // draw cursor.
                int center_x = GetScreenWidth() / 2;
                int center_y = GetScreenHeight() / 2;

                DrawPixel(center_x, center_y, WHITE);


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
            }

        }
        EndDrawing();

        handle_userinput(gst);
        update_player(gst, &gst->player);
    }



    //UnloadModel(testfloor);
}



void cleanup(struct state_t* gst) {
    
    for(unsigned int i = 0; i < gst->num_textures; i++) {
        UnloadTexture(gst->tex[i]);
    }

    free_objarray(gst);
    //delete_psystem(&gst->psystems[PSYS_ENEMYHIT]);
    //free_enemyarray(gst);
    free_player(&gst->player);
    UnloadShader(gst->shaders[DEFAULT_SHADER]);
    UnloadShader(gst->shaders[PLAYER_PROJECTILE_SHADER]);
    UnloadShader(gst->shaders[PLANET_SKYBOX_SHADER]);
    //UnloadShader(gst->shaders[ENEMY_HIT_PSYS_SHADER]);
    
    delete_terrain(&gst->terrain);
    UnloadMesh(gst->skybox_mesh);

    CloseWindow();
}


void load_tex(struct state_t* gst, const char* filepath, int texid) {
    
    if(texid >= MAX_TEXTURES) {
        fprintf(stderr, "ERROR: Texture id is invalid. (%i) for '%s'\n", texid, filepath);
        return;
    }
    gst->tex[texid] = LoadTexture(filepath);
    gst->num_textures++;

    printf("\033[32m + Loaded (texid:%i) '%s'\033[0m \n", texid, filepath);
}

void first_setup(struct state_t* gst) {


    InitWindow(SCRN_W, SCRN_H, "Game");


    // --- misc. ---
    DisableCursor();
    SetTargetFPS(500);
    gst->num_textures = 0;
    gst->num_lights = 0;
    gst->draw_debug = 0;
    
    gst->objects = NULL;
    gst->objarray_size = 0;
    gst->num_objects = 0;
  
    gst->next_projlight_index = 0;
    /*
    gst->enemies = NULL;
    gst->enemyarray_size = 0;
    gst->num_enemies = 0;
    gst->next_projlight_index = 0;
    */

    gst->dt = 0.016;

    memset(gst->fs_unilocs, 0, MAX_FS_UNILOCS * sizeof *gst->fs_unilocs);

    const float fog_density = 0.003;
    const float terrain_scale = 8.0;
    const u32   planet_size = 500;
    const float terrain_amplitude = 30.0;
    const float terrain_pnfrequency = 10.0;
    const int   terrain_octaves = 3;
    const Vector3 sun_position = (Vector3) { 0.0, 0.5, -0.9 };


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
    // -----------------------


    // --- Setup Projectile Shader ---
    {
        Shader* shader = &gst->shaders[PLAYER_PROJECTILE_SHADER];
        *shader = LoadShader(
            "res/shaders/projectile.vs", 
            "res/shaders/projectile.fs"
        );


        gst->fs_unilocs[PLAYER_PROJECTILE_EFFECTSPEED_FS_UNILOC] 
            = GetShaderLocation(*shader, "effect_speed");

    }
 
    // --- Setup Skybox Shader ---
    {
        Shader* shader = &gst->shaders[PLANET_SKYBOX_SHADER];
        *shader = LoadShader(
            "res/shaders/sky.vs", 
            "res/shaders/sky.fs"
        );

        gst->fs_unilocs[PLANET_SUN_POSITION_FS_UNILOC]
            = GetShaderLocation(*shader, "sun_position");
        
        gst->fs_unilocs[PLANET_SKYBOX_VIEWPOS_FS_UNILOC]
            = GetShaderLocation(*shader, "view_position");
        
        gst->fs_unilocs[PLANET_SKYBOX_GLOBALTIME_FS_UNILOC]
            = GetShaderLocation(*shader, "time");

        float sunpos[4] = { 
            sun_position.x,
            sun_position.y,
            sun_position.z
        };
        SetShaderValue(*shader, gst->fs_unilocs[PLANET_SUN_POSITION_FS_UNILOC], sunpos, SHADER_UNIFORM_VEC3);
    
    }
 
    
    /*

    // --- Setup (ENEMY_HIT) ParticleSystem Shader ---
    {
        Shader* shader = &gst->shaders[ENEMY_HIT_PSYS_SHADER];
        *shader = LoadShader(
            "res/shaders/particle_core.vs",
            "res/shaders/enemy_hit_psystem.fs"
        );
       
        shader->locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(*shader, "mvp");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
       
    }
    // -----------------------


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

        psys->particle_material = LoadMaterialDefault();
        psys->particle_material.shader = gst->shaders[ENEMY_HIT_PSYS_SHADER];
        psys->particle_mesh = GenMeshCube(0.2, 0.2, 0.2);
    }
    */

    // make sure all lights are disabled
    for(int i = 0; i < (MAX_LIGHTS + MAX_PROJECTILE_LIGHTS); i++) {
        int enabled = 0;
        int loc = GetShaderLocation(gst->shaders[DEFAULT_SHADER], TextFormat("lights[%i].enabled", i));
        SetShaderValue(gst->shaders[DEFAULT_SHADER], loc, &enabled, SHADER_UNIFORM_INT);
        
        loc = GetShaderLocation(gst->shaders[DEFAULT_SHADER], TextFormat("projlights[%i].enabled", i));
        SetShaderValue(gst->shaders[DEFAULT_SHADER], loc, &enabled, SHADER_UNIFORM_INT);
    }

    // --- load textures. ---
    
    load_tex(gst, "res/textures/grid_4x4.png", GRID4x4_TEXID);
    load_tex(gst, "res/textures/grid_6x6.png", GRID6x6_TEXID);
    load_tex(gst, "res/textures/grid_9x9.png", GRID9x9_TEXID);
    load_tex(gst, "res/textures/enemy.png", ENEMY_0_TEXID);
    load_tex(gst, "res/textures/gun_0.png", GUN_0_TEXID);

    init_player_struct(gst, &gst->player);



    // --- Setup Terrain ----
    {
        init_perlin_noise();
        gst->terrain = (struct terrain_t) { 0 };


        generate_terrain(
                gst, &gst->terrain,
                planet_size,
                terrain_scale,
                terrain_amplitude,
                terrain_pnfrequency,
                terrain_octaves
                );

        /*
        generate_heightmap(&gst->terrain);
        generate_terrain_mesh(gst, &gst->terrain);

        */

    }

    // --- Setup skybox cube ---
    // Create big cube around the whole planet.
    {
        gst->skybox_mesh = GenMeshSphere(1, 8, 8);
        gst->skybox_material = LoadMaterialDefault();
        gst->skybox_material.shader = gst->shaders[PLANET_SKYBOX_SHADER];

        gst->skybox_transform = MatrixTranslate(0, 0, 0);
    }


    // --- add lights. ---
    
    gst->lights[gst->num_lights] 
        = CreateLight(gst, LIGHT_POINT, 
                sun_position, (Vector3){0,0,0},
                (Color){ 200, 100, 30, 255 }, gst->shaders[DEFAULT_SHADER]);



    int seed = time(0);

    gst->rseed = seed;
    SetRandomSeed(seed);

}


int main(void) {

    struct state_t gst;

    first_setup(&gst);
    loop(&gst);
    cleanup(&gst);


    printf("Exit 0.\n");
    return 0;
}
