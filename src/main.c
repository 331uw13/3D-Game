

#include <raylib.h>
#include <stdio.h>
#include <math.h>
#include <raymath.h>

#include "state.h"
#include "input.h"
#include "util.h"

#define SCRN_W 800
#define SCRN_H 600

#define GLSL_VERSION 330


void cleanup(struct state_t* gst);




int check_player_collision(struct state_t* gst, struct box_t* box) {
    int res = 0;

    struct box_t playerbox = { 
        gst->cam.position,
        gst->player_size
    };

    


    return res;
}



void loop(struct state_t* gst) {


    Model testcube = LoadModelFromMesh(GenMeshCube(2.0, 2.0, 2.0));
    Model testfloor = LoadModelFromMesh(GenMeshCube(40.0, 0.25, 40.0));

    Model testmodel = LoadModel("res/models/street-lamp.glb");


    testcube.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = gst->tex[GRID4x4_TEXID];
    testcube.materials[0].shader = gst->light_shader;

    testfloor.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = gst->tex[GRID9x9_TEXID];
    testfloor.materials[0].shader = gst->light_shader;

    testmodel.materials[0].shader = gst->light_shader;
    testmodel.transform = MatrixTranslate(-5.0, -2.0, -4.0);

    Color testcube_color = { 255.0, 0.0, 0.0, 255 };


    while(!WindowShouldClose()) {
        float dt = GetFrameTime();
        double time = GetTime();

        // --- update movement. ---

        /*
        struct box_t floorbox = {
            (Vector3) { 0.0, -2.0, 0.0 },
            (Vector3) { 40.0, 0.25, 40.0 }
        };

        int hit = check_player_collision(gst, &floorbox);      
        */

        handle_userinput(gst);

       


        // --- update lights. ---
        
        float camposf3[3] = { gst->cam.position.x, gst->cam.position.y, gst->cam.position.z };
        SetShaderValue(gst->light_shader, 
                gst->light_shader.locs[SHADER_LOC_VECTOR_VIEW], camposf3, SHADER_UNIFORM_VEC3);


        for(int i = 0; i < gst->num_lights; i++) {
            UpdateLightValues(gst->light_shader, gst->lights[i]);
        }



        // --- misc. ---

        testcube.transform = MatrixMultiply(testcube.transform, MatrixRotateZ(-0.012));
        testcube.transform = MatrixMultiply(testcube.transform, MatrixRotateY(-0.012));
        testcube.transform = MatrixMultiply(testcube.transform, MatrixRotateX(-0.012));





        // --- render. ---

        BeginDrawing();
        {
            ClearBackground((Color){ 20, 20, 20, 255 });


            // Draw 3D stuff
            BeginMode3D(gst->cam);
            {
                BeginShaderMode(gst->light_shader);


                rainbow_palette(sin(time), &testcube_color.r, &testcube_color.g, &testcube_color.b);

                DrawModel(testcube,(Vector3){ 0.0, 0.0, 8.0 }, 1.0, testcube_color);
                DrawModel(testfloor, (Vector3){ 0.0, -2.0, 0.0 }, 1.0, WHITE);
            
                DrawMesh(testmodel.meshes[0], testmodel.materials[0], testmodel.transform);


            

                EndShaderMode();


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

            }
            EndMode3D();


            // Draw 2D stuff
            {

                // draw cursor.
                int center_x = GetScreenWidth() / 2;
                int center_y = GetScreenHeight() / 2;

                DrawPixel(center_x, center_y, WHITE);



                DrawText(TextFormat("FPS(%i)", GetFPS()),
                        15.0, 10.0, 20.0, PURPLE);

                DrawText("-- Velocity,",
                        15.0, 38.0, 20.0, BLUE
                        );
                DrawText(TextFormat("x: %0.3f", gst->player_velocity.x),
                        40.0, 60.0, 20.0, BLUE);
                DrawText(TextFormat("y: %0.3f", gst->player_velocity.y),
                        40.0, 60.0+20, 20.0, BLUE);
                DrawText(TextFormat("z: %0.3f", gst->player_velocity.z),
                        40.0, 60.0+40, 20.0, BLUE);



                DrawText("-- Position,",
                        15.0, 120.0, 20.0, RED
                        );
                DrawText(TextFormat("x: %0.3f", gst->cam.position.x),
                        40.0, 120.0+20, 20.0, RED);
                DrawText(TextFormat("y: %0.3f", gst->cam.position.y),
                        40.0, 120.0+40, 20.0, RED);
                DrawText(TextFormat("z: %0.3f", gst->cam.position.z),
                        40.0, 120.0+60, 20.0, RED);



            }

        }
        EndDrawing();
    }

    UnloadModel(testcube);
    UnloadModel(testfloor);
    UnloadModel(testmodel);

}



void cleanup(struct state_t* gst) {
    
    for(unsigned int i = 0; i < gst->num_textures; i++) {
        UnloadTexture(gst->tex[i]);
    }

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
    gst->cam.position = (Vector3){ 0.0, 1.0, 0.0 };
    gst->cam.target = (Vector3){ 0.0, 0.0, 1.0 };
    gst->cam.up = (Vector3){ 0.0, 1.0, 0.0 };
    gst->cam.fovy = 80.0;
    gst->cam.projection = CAMERA_PERSPECTIVE;


    // --- setup player. ---
    gst->player_size = (Vector3){ 1.0, 2.0, 1.0 };
    gst->player_velocity = (Vector3) { 0 };
    gst->player_walkspeed = 0.8;
    gst->player_jump_force = 0.126;
    gst->player_mass = 1.0;
    gst->player_gravity = 0.6;
    gst->player_run_mult = 2.3;
    gst->player_onground = 1;


    // --- misc. ---
    DisableCursor();
    SetTargetFPS(120);
    gst->num_textures = 0;
    gst->num_lights = 0;

    // --- setup shaders. ---
    
    gst->light_shader 
        = LoadShader(
            TextFormat("res/shaders/lighting.vs", GLSL_VERSION),
            TextFormat("res/shaders/fog.fs", GLSL_VERSION)
            );

    gst->light_shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(gst->light_shader, "matModel");
    gst->light_shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(gst->light_shader, "viewPos");
    int ambientloc = GetShaderLocation(gst->light_shader, "ambient");
    SetShaderValue(gst->light_shader, ambientloc, (float[4]){ 0.1, 0.1, 0.1, 1.0}, SHADER_UNIFORM_VEC4);

    
    // --- load textures. ---

    
    load_tex(gst, "res/textures/grid_4x4.png", GRID4x4_TEXID);
    load_tex(gst, "res/textures/grid_6x6.png", GRID6x6_TEXID);
    load_tex(gst, "res/textures/grid_9x9.png", GRID9x9_TEXID);



    // --- add lights. ---
    
    gst->lights[gst->num_lights] 
        = CreateLight(gst, LIGHT_POINT, 
                (Vector3){ -4.05, 2.01, -4.0 }, (Vector3){0,0,0},
                (Color){ 200, 100, 30, 255 }, gst->light_shader);



}


int main(void) {

    struct state_t gst;

    first_setup(&gst);
    loop(&gst);
    cleanup(&gst);


    printf("Exit 0.\n");
    return 0;
}
