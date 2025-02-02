

#include <raylib.h>
#include <stdio.h>
#include <math.h>
#include <raymath.h>
#include <time.h>

#include "state.h"
#include "input.h"
#include "util.h"

#define SCRN_W 1200
#define SCRN_H 700

#define GLSL_VERSION 330


void cleanup(struct state_t* gst);




void testpsys_pupdate(struct state_t* gst, struct psystem_t* psys, struct particle_t* p) {

}

void testpsys_pinit(struct state_t* gst, struct psystem_t* psys, struct particle_t* p) {

}


void loop(struct state_t* gst) {

    
    Model testfloor = LoadModelFromMesh(GenMeshCube(40.0, 0.25, 40.0));
    testfloor.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = gst->tex[GRID9x9_TEXID];
    testfloor.materials[0].shader = gst->shaders[DEFAULT_SHADER];


    create_object(gst, "res/models/street-lamp.glb", NONE_TEXID, (Vector3){ -3.0, 0.0, 0.0 });
    
    create_enemy(gst,
            "res/models/enemy.glb", ENEMY_0_TEXID,
            ENEMY_0_MAX_HEALTH,
            (Vector3) { 2.0, 2.0, 2.0 }, /* hitbox */
            (Vector3) { -6.0, 2.0, 3.0 } /* position */

            );

    create_enemy(gst,
            "res/models/enemy.glb", ENEMY_0_TEXID,
            ENEMY_0_MAX_HEALTH,
            (Vector3) { 2.0, 2.0, 2.0 }, /* hitbox */
            (Vector3) { 6.0, 2.0, -3.0 } /* position */

            );

    create_enemy(gst,
            "res/models/enemy.glb", ENEMY_0_TEXID,
            ENEMY_0_MAX_HEALTH,
            (Vector3) { 2.0, 2.0, 2.0 }, /* hitbox */
            (Vector3) { 6.0, 0.0, -7.0 } /* position */

            );


    // Setup shader for particle system. ---------
    //
  /*
    Shader testshader = LoadShader(
            "lighting_instancing.vs",
            "res/shaders/particle.fs"
            );
   
    testshader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(testshader, "mvp");
    testshader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(testshader, "viewPos");
    testshader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(testshader, "instanceTransform");
    
    */


    // Setup material for particle system. -------
    //
    Material testmaterial = LoadMaterialDefault();
    testmaterial.shader = gst->shaders[TEST_PSYS_SHADER];

    // Setup mesh for particles. -----------------
    //
    Mesh testmesh = GenMeshCube(0.3, 0.3, 0.3);


    // Create the particle system ----------------

    struct psystem_t testpsys;
    create_psystem(
            gst,
            &testpsys,
            10000,
            testpsys_pupdate,
            testpsys_pinit
            );

    testpsys.particle_material = testmaterial;
    testpsys.particle_mesh = testmesh;


    printf("%i\n", testpsys.particle_material.shader.locs[SHADER_LOC_MATRIX_MODEL]);

    while(!WindowShouldClose()) {
        float dt = GetFrameTime();
        double time = GetTime();

        gst->dt = dt;

        // --- update movement. ---

        handle_userinput(gst);
        update_player(gst, &gst->player);
       

        
        float camposf3[3] = { gst->player.cam.position.x, gst->player.cam.position.y, gst->player.cam.position.z };
        SetShaderValue(gst->shaders[DEFAULT_SHADER], 
                gst->shaders[DEFAULT_SHADER].locs[SHADER_LOC_VECTOR_VIEW], camposf3, SHADER_UNIFORM_VEC3);

        
        SetShaderValue(gst->shaders[TEST_PSYS_SHADER], 
                gst->shaders[TEST_PSYS_SHADER].locs[SHADER_LOC_VECTOR_VIEW], camposf3, SHADER_UNIFORM_VEC3);
        


        // --- render. ---

        BeginDrawing();
        {
            ClearBackground((Color){ 20, 20, 20, 255 });



            // Draw 3D stuff
            BeginMode3D(gst->player.cam);
            {
                
                update_psystem(gst, &testpsys);
           

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


                // Draw enemies.
                //
                for(size_t i = 0; i < gst->num_enemies; i++) {
                    struct enemy_t* enemy = &gst->enemies[i];
                    update_enemy(gst, enemy);

                    DrawMesh(
                            enemy->model.meshes[0],
                            enemy->model.materials[0],
                            enemy->model.transform
                            );


                    
                }


                DrawModel(testfloor, (Vector3){ 0.0, 0.0, 0.0 }, 1.0, WHITE);

                player_render(gst, &gst->player);



    
                EndShaderMode();
               




            }
            EndMode3D();


            // Draw 2D stuff
            {

                // draw cursor.
                int center_x = GetScreenWidth() / 2;
                int center_y = GetScreenHeight() / 2;

                DrawPixel(center_x, center_y, WHITE);



                DrawText(TextFormat("FPS(%i)", GetFPS()),
                        15.0, 10.0, 20.0, WHITE);


                DrawText(TextFormat("DrawDebug (%s)", gst->draw_debug ? "ON" : "OFF"),
                        15.0, GetScreenHeight() - 30.0, 20.0, (Color){80,150,160,255});

            }

        }
        EndDrawing();
    }

    UnloadMesh(testmesh);
    delete_psystem(&testpsys);

    UnloadModel(testfloor);
}



void cleanup(struct state_t* gst) {
    
    for(unsigned int i = 0; i < gst->num_textures; i++) {
        UnloadTexture(gst->tex[i]);
    }

    free_objarray(gst);
    free_enemyarray(gst);
    free_player(&gst->player);
    UnloadShader(gst->shaders[DEFAULT_SHADER]);
    UnloadShader(gst->shaders[TEST_PSYS_SHADER]);
    UnloadShader(gst->player.gun.projectile_shader);
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
    SetTargetFPS(120);
    gst->num_textures = 0;
    gst->num_lights = 0;
    gst->draw_debug = 0;
    
    gst->objects = NULL;
    gst->objarray_size = 0;
    gst->num_objects = 0;
    
    gst->enemies = NULL;
    gst->enemyarray_size = 0;
    gst->num_enemies = 0;
    gst->next_projlight_index = 0;

  


    gst->player.gun.projectile_shader
        = LoadShader(
                "res/shaders/projectile.vs", 
                "res/shaders/projectile.fs"
            );

    
    float fog_density = 0.045;


    // --- Setup Default Shader ---
    {
  
        Shader* shader = &gst->shaders[DEFAULT_SHADER];

        *shader = LoadShader(
                "res/shaders/lighting.vs",
                "res/shaders/fog.fs"
            );

        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(*shader, "matModel");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
   
        int ambientloc = GetShaderLocation(*shader, "ambient");
        int fogdensityloc = GetShaderLocation(*shader, "fogDensity");
        
        SetShaderValue(*shader, ambientloc, (float[4]){ 0.5, 0.5, 0.5, 1.0}, SHADER_UNIFORM_VEC4);
        SetShaderValue(*shader, fogdensityloc, &fog_density, SHADER_UNIFORM_FLOAT);
    }
    // -----------------------



    // --- Setup Test ParticleSystem Shader ---
    {
        Shader* shader = &gst->shaders[TEST_PSYS_SHADER];
        *shader = LoadShader(
                "res/shaders/particle_core.vs",
                "res/shaders/test_psystem.fs"
                );
       
        shader->locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(*shader, "mvp");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
       

    }
    // -----------------------


    // make all lights disabled
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

    // --- add lights. ---
    
    gst->lights[gst->num_lights] 
        = CreateLight(gst, LIGHT_DIRECTIONAL, 
                (Vector3){ -4.05, 2.01, -4.0 }, (Vector3){0,0,0},
                (Color){ 200, 100, 30, 255 }, gst->shaders[DEFAULT_SHADER]);


    SetRandomSeed(time(0));

}


int main(void) {

    struct state_t gst;

    first_setup(&gst);
    loop(&gst);
    cleanup(&gst);


    printf("Exit 0.\n");
    return 0;
}
