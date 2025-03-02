
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

        SetShaderValue(
                gst->shaders[PROJECTILES_ENVHIT_SHADER],
                gst->fs_unilocs[PROJECTILES_ENVHIT_TIME_FS_UNILOC],
                &gst->time,
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

    // One enemy type uses the same gun.
    weapon_update(gst, &gst->entity_weapons[ENTWEAPON_LVL0]);


    // Particle systems.
    update_psystem(gst, &gst->psystems[PROJECTILE_ENVHIT_PSYSTEM]);
    update_psystem(gst, &gst->psystems[PROJECTILE_ELVL0_ENVHIT_PSYSTEM]);
    update_psystem(gst, &gst->psystems[PROJECTILE_ENTITYHIT_PSYSTEM]);
}



void state_render_environment(struct state_t* gst) {


    // Render 3D stuff into texture and post process it later.
    BeginTextureMode(gst->env_render_target);
    ClearBackground((Color){(0.3) * 255, (0.15) * 255, (0.15) * 255, 255 });
    BeginMode3D(gst->player.cam);
    {

        // Render debug info if needed.
        if(gst->debug) {
            for(size_t i = 0; i < gst->num_entities; i++) {
                struct entity_t* ent = &gst->entities[i];
      
                if(gst->debug) {          
                    DrawBoundingBox(get_entity_boundingbox(ent), RED);
                }
            }
        }
 

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
        weapon_render_projectiles(gst, &gst->entity_weapons[ENTWEAPON_LVL0]);

        // Entities.
        for(size_t i = 0; i < gst->num_entities; i++) {
            struct entity_t* ent = &gst->entities[i];
            render_entity(gst, ent);
      
        }

        // Player.
        player_render(gst, &gst->player);


        // Particle systems.
   


        // FOR TESTING --  MOVE LATER ------


        float psystem_color[3] = {
            (float)gst->player.weapon.prj_color.r / 255.0,
            (float)gst->player.weapon.prj_color.g / 255.0,
            (float)gst->player.weapon.prj_color.b / 255.0
        };

        SetShaderValue(
            gst->shaders[PROJECTILES_ENVHIT_SHADER], 
            gst->fs_unilocs[PROJECTILES_ENVHIT_COLOR_FS_UNILOC],
            psystem_color,
            SHADER_UNIFORM_VEC3
            );

        SetShaderValue(
            gst->shaders[PROJECTILES_PSYSTEM_SHADER], 
            gst->fs_unilocs[PROJECTILES_PSYSTEM_COLOR_FS_UNILOC],
            psystem_color,
            SHADER_UNIFORM_VEC3
            );

        render_psystem(gst, &gst->psystems[PROJECTILE_ENVHIT_PSYSTEM]);



        float psystem3_color[3] = {
            (float)gst->entity_weapons[ENTWEAPON_LVL0].prj_color.r / 255.0,
            (float)gst->entity_weapons[ENTWEAPON_LVL0].prj_color.g / 255.0,
            (float)gst->entity_weapons[ENTWEAPON_LVL0].prj_color.b / 255.0
        };

        SetShaderValue(
            gst->shaders[PROJECTILES_ENVHIT_SHADER], 
            gst->fs_unilocs[PROJECTILES_ENVHIT_COLOR_FS_UNILOC],
            psystem3_color,
            SHADER_UNIFORM_VEC3
            );

        render_psystem(gst, &gst->psystems[PROJECTILE_ELVL0_ENVHIT_PSYSTEM]);







        float psystem2_color[3] = {
            (float)255 / 255.0,
            (float)120 / 255.0,
            (float)50 / 255.0
        };

        SetShaderValue(
            gst->shaders[PROJECTILES_PSYSTEM_SHADER], 
            gst->fs_unilocs[PROJECTILES_PSYSTEM_COLOR_FS_UNILOC],
            psystem2_color,
            SHADER_UNIFORM_VEC3
            );
        
        
        render_psystem(gst, &gst->psystems[PROJECTILE_ENTITYHIT_PSYSTEM]);
        
        // ------------------------

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



