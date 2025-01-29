

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



void loop(struct state_t* gst) {

    
    Model testfloor = LoadModelFromMesh(GenMeshCube(40.0, 0.25, 40.0));
    testfloor.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = gst->tex[GRID9x9_TEXID];
    testfloor.materials[0].shader = gst->light_shader;


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



    // TODO: light management?
    for(int i = 0; i < gst->num_lights; i++) {
        UpdateLightValues(gst->light_shader, gst->lights[i]);
    }


    while(!WindowShouldClose()) {
        float dt = GetFrameTime();
        double time = GetTime();

        gst->dt = dt;

        // --- update movement. ---

        handle_userinput(gst);
        update_player(gst, &gst->player);
       


        // --- update lights. ---
        
        float camposf3[3] = { gst->cam.position.x, gst->cam.position.y, gst->cam.position.z };
        SetShaderValue(gst->light_shader, 
                gst->light_shader.locs[SHADER_LOC_VECTOR_VIEW], camposf3, SHADER_UNIFORM_VEC3);


        // --- render. ---

        BeginDrawing();
        {
            ClearBackground((Color){ 20, 20, 20, 255 });


            // Draw 3D stuff
            BeginMode3D(gst->cam);
            {
                BeginShaderMode(gst->light_shader);

                // draw objects
                for(size_t i = 0; i < gst->num_objects; i++) {
                    struct obj_t* obj = &gst->objects[i];
                    DrawMesh(
                            obj->model.meshes[0],
                            obj->model.materials[0],
                            obj->model.transform
                            );
                
                }

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


                EndShaderMode();


                //DrawSphere(gst->enemies[0].travel_dest, 0.4, RED);


                /*
                for(int i = 0; i < gst->num_lights; i++) {
                    DrawSphere(gst->lights[i].position, 0.1, WHITE);
                    
                    float rad = 0.2;
                    float remove = 100 / 3;
                    float alpha = 100;
                    for(int k = 0; k < 3; k++) {
                        DrawSphere(gst->lights[i].position, rad, (Color){255, 180, 40, alpha });
                        alpha -= remove;
                        rad += 0.1;
                    }
                }
                */

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

    UnloadModel(testfloor);
}



void cleanup(struct state_t* gst) {
    
    for(unsigned int i = 0; i < gst->num_textures; i++) {
        UnloadTexture(gst->tex[i]);
    }

    free_objarray(gst);
    free_enemyarray(gst);
    UnloadShader(gst->light_shader);
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

    // --- setup camera for 3D world. ---
    gst->cam = (Camera){ 0 };
    gst->cam.position = (Vector3){ 0.0, 3.0, 0.0 };
    gst->cam.target = (Vector3){ 0.0, 0.0, 1.0 };
    gst->cam.up = (Vector3){ 0.0, 1.0, 0.0 };
    gst->cam.fovy = 80.0;
    gst->cam.projection = CAMERA_PERSPECTIVE;


    init_player_struct(&gst->player);

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


    // --- setup shaders. ---
    
    gst->light_shader 
        = LoadShader(
            TextFormat("res/shaders/lighting.vs", GLSL_VERSION),
            TextFormat("res/shaders/fog.fs", GLSL_VERSION)
            );

    gst->light_shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(gst->light_shader, "matModel");
    gst->light_shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(gst->light_shader, "viewPos");
    int ambientloc = GetShaderLocation(gst->light_shader, "ambient");
    int fogdensityloc = GetShaderLocation(gst->light_shader, "fogDensity");
    SetShaderValue(gst->light_shader, ambientloc, (float[4]){ 0.5, 0.5, 0.5, 1.0}, SHADER_UNIFORM_VEC4);
    float fog_density = 0.045;
    SetShaderValue(gst->light_shader, fogdensityloc, &fog_density, SHADER_UNIFORM_FLOAT);
    
    // --- load textures. ---
    
    load_tex(gst, "res/textures/grid_4x4.png", GRID4x4_TEXID);
    load_tex(gst, "res/textures/grid_6x6.png", GRID6x6_TEXID);
    load_tex(gst, "res/textures/grid_9x9.png", GRID9x9_TEXID);
    load_tex(gst, "res/textures/enemy.png", ENEMY_0_TEXID);



    // --- add lights. ---
    
    gst->lights[gst->num_lights] 
        = CreateLight(gst, LIGHT_DIRECTIONAL, 
                (Vector3){ -4.05, 2.01, -4.0 }, (Vector3){0,0,0},
                (Color){ 200, 100, 30, 255 }, gst->light_shader);


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
