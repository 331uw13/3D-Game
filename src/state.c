
#include "state.h"
#include "input.h"
#include <raymath.h>



void state_update_shader_uniforms(struct state_t* gst) {


    // Update Player view position.
    //
    {
        float camposf3[3] = { gst->player.cam.position.x, gst->player.cam.position.y, gst->player.cam.position.z };
        
        SetShaderValue(gst->shaders[DEFAULT_SHADER], 
                gst->shaders[DEFAULT_SHADER].locs[SHADER_LOC_VECTOR_VIEW], camposf3, SHADER_UNIFORM_VEC3);

                
        SetShaderValue(gst->shaders[PROJECTILES_PSYSTEM_SHADER], 
                gst->shaders[PROJECTILES_PSYSTEM_SHADER].locs[SHADER_LOC_VECTOR_VIEW], camposf3, SHADER_UNIFORM_VEC3);

    }


    // Update screen size.
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


    // Misc.
    {

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
    }
}


// NOTE: DO NOT RENDER FROM HERE:
void state_update_frame(struct state_t* gst) {
    
    handle_userinput(gst);
    player_update(gst, &gst->player);
    weapon_update(gst, &gst->player.weapon);


    // Entities.
    for(size_t i = 0; i < gst->num_entities; i++) {
        update_entity(gst, &gst->entities[i]);
    }

}



void state_render_environment(struct state_t* gst) {


    // Render 3D stuff into texture and post process it later.
    BeginTextureMode(gst->env_render_target);
    ClearBackground((Color){(0.3) * 255, (0.15) * 255, (0.15) * 255, 255 });
    BeginMode3D(gst->player.cam);
    {

        BeginShaderMode(gst->shaders[DEFAULT_SHADER]);


        DrawCube((Vector3){ 20.0, 3.0, -10.0 }, 3.0, 3.0, 3.0, (Color){ 30, 30, 30, 255});

        /*
        // Test thing.
        {
            testmodel.transform = MatrixRotateXYZ((Vector3){ gst->time, 0.0, gst->time });
            DrawModel(testmodel, (Vector3){ 5.0, 5.0, -5.0 }, 1.0, (Color){ 255, 255, 255, 255});
        }
        */

        // Terrain.
        render_terrain(gst, &gst->terrain);
        
        // Weapon projectiles.
        weapon_render_projectiles(gst, &gst->player.weapon);

        // Entities.
        for(size_t i = 0; i < gst->num_entities; i++) {
            render_entity(gst, &gst->entities[i]);
        }

        // Player.
        player_render(gst, &gst->player);

        EndShaderMode();
    }
    EndMode3D();
    EndTextureMode();
    EndShaderMode();



    // Get bloom treshold texture.

    BeginTextureMode(gst->bloomtreshold_target);
    ClearBackground((Color){0,0,0, 255 });
    BeginShaderMode(gst->shaders[BLOOM_TRESHOLD_SHADER]);
    {
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
    EndTextureMode();

   
}



