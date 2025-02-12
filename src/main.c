

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


#define PFORCE 0.0
#define PMASS  100.0
#define GRAVITYPOINT_MASS  5.0
#define GRAVITY 0.00001

static Vector3 gravity_point = (Vector3) { 20.0, 20.0, 0.0 };

Vector3 initvec3_v(float v) {
    return (Vector3) { v, v, v };
}

Vector3 vec3div_v(Vector3 a, float v) {
    return (Vector3) { a.x/v, a.y/v, a.z/v };
}

Vector3 vec3mult_v(Vector3 a, float v) {
    return (Vector3) { a.x*v, a.y*v, a.z*v };
}

Vector3 vec3add_v(Vector3 a, float v) {
    return (Vector3) { a.x+v, a.y+v, a.z+v };
}


static int g_testpsys_point_loc; 



void testpsys_pupdate(struct state_t* gst, struct psystem_t* psys, struct particle_t* p) {

    const Vector3 pmass = initvec3_v(PMASS);
    const Vector3 gp_mass = initvec3_v(GRAVITYPOINT_MASS);


    // Direction to gravitation point.
    Vector3 dir = Vector3Subtract(p->position, gravity_point);
    float mag = Vector3LengthSqr(dir);


    float strength = GRAVITY * ((PMASS * GRAVITYPOINT_MASS) / mag);
    strength += 0.0132;

    dir = vec3set_mag(dir, strength);

    dir = vec3div_v(dir, 20.0);
    dir = Vector3Negate(dir);
    p->accel = Vector3Add(p->accel, dir);
    
    
    //p->velocity = Vector3Add(p->velocity, p->accel);
    p->position = Vector3Add(p->position, p->accel);

    
    *p->transform = MatrixTranslate(p->position.x, p->position.y, p->position.z);


}

static float newx = 0.0;
static float newz = 0.0;
static float newy = 0.0;
static float incr = 0.0;

void testpsys_pinit(struct state_t* gst, struct psystem_t* psys, struct particle_t* p) {

    p->position = (Vector3)
    { 
        RSEEDRANDOMF(-2.0, 2.0) + newx * 10.0,
        RSEEDRANDOMF(-2.0, 2.0) + newy * 10.0 + 30.0,
        RSEEDRANDOMF(-2.0, 2.0) + newz * 10.0
    };

    float t = gst->dt;
    p->velocity = (Vector3)
    { 
        RSEEDRANDOMF(-0.1, 0.1),
        RSEEDRANDOMF(-0.1, 0.1),
        RSEEDRANDOMF(-0.1, 0.1)
    };
    p->color = RED;

    p->lifetime = 0.0;
    p->max_lifetime = RSEEDRANDOMF(0.25, 3.25);
    p->alive = 1;


    incr += 0.3;
    float pi2 = M_PI*1.23;

    newx = cos(newx*pi2) * cos(incr*M_PI);
    newy = cos(newz*pi2);
    newz = sin(newx*pi2) * cos(incr*M_PI);


    *p->transform = MatrixTranslate(p->position.x, p->position.y, p->position.z);
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
            (Vector3) { -12.0, 2.0, 3.0 } /* position */

            );

    create_enemy(gst,
            "res/models/enemy.glb", ENEMY_0_TEXID,
            ENEMY_0_MAX_HEALTH,
            (Vector3) { 2.0, 2.0, 2.0 }, /* hitbox */
            (Vector3) { 12.0, 2.0, 8.0 } /* position */

            );

    create_enemy(gst,
            "res/models/enemy.glb", ENEMY_0_TEXID,
            ENEMY_0_MAX_HEALTH,
            (Vector3) { 2.0, 2.0, 2.0 }, /* hitbox */
            (Vector3) { 12.0, 0.0, -7.0 } /* position */

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
    const float tmsize = 0.1;
    Mesh testmesh = GenMeshCube(tmsize, tmsize, tmsize);


    // Create the particle system ----------------

    Model particlemodel = LoadModel("res/models/particle.glb");

    struct psystem_t testpsys;
    create_psystem(
            gst,
            &testpsys,
            150000,
            testpsys_pupdate,
            testpsys_pinit
            );

    testpsys.particle_material = testmaterial;
    testpsys.particle_mesh = testmesh;




    add_particles(gst, &testpsys, 150000);



    // ---------------------------------------------

    

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
       

    
        if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            const float dist = 3.0;
            Vector3 point = gst->player.position;

            add_movement_vec3(&point, gst->player.looking_at, 
                    Vector3Distance(gravity_point, point)
                    );

            gravity_point = point;
        }
       
        {
            float point[3] = {
                gravity_point.x, gravity_point.y, gravity_point.z
            };
            SetShaderValue(gst->shaders[TEST_PSYS_SHADER], g_testpsys_point_loc, 
                point, SHADER_UNIFORM_VEC3);

        }
        // --- render. ---

        BeginDrawing();
        {
            ClearBackground((Color){ 10, 10, 10, 255 });

    


            // Draw 3D stuff
            BeginMode3D(gst->player.cam);
            {
                
           

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
                DrawSphere(gravity_point, 0.3, (Color){ 0.0, 200, 200, 150 });
               

                //update_psystem(gst, &testpsys);
                update_psystem(gst, &testpsys);



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

                DrawText(TextFormat("NoClip(%s)", gst->player.noclip ? "ON" : "OFF"),
                        120.0, 10.0, 20.0, GREEN);

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

    gst->dt = 0.016;
  


    gst->player.gun.projectile_shader
        = LoadShader(
                "res/shaders/projectile.vs", 
                "res/shaders/projectile.fs"
            );

    
    float fog_density = 0.040;


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
       
        g_testpsys_point_loc = GetShaderLocation(*shader, "gravity_point");
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
