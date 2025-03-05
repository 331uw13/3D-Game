
#include "state.h"
#include "input.h"
#include "util.h"

#include <raymath.h>


#include "particle_systems/weapon_psys.h"
#include "particle_systems/prj_envhit_psys.h"
#include "particle_systems/enemy_hit_psys.h"


void state_update_shader_uniforms(struct state_t* gst) {


    // Update Player view position.
    //
    {
        float camposf3[3] = { gst->player.cam.position.x, gst->player.cam.position.y, gst->player.cam.position.z };
        
        SetShaderValue(gst->shaders[DEFAULT_SHADER], 
                gst->shaders[DEFAULT_SHADER].locs[SHADER_LOC_VECTOR_VIEW], camposf3, SHADER_UNIFORM_VEC3);

     
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

}


// NOTE: DO NOT RENDER FROM HERE:
void state_update_frame(struct state_t* gst) {
    
    handle_userinput(gst);
    player_update(gst, &gst->player);


    // Enemies.
    for(size_t i = 0; i < gst->num_enemies; i++) {
        update_enemy(gst, &gst->enemies[i]);
    }


    update_psystem(gst, &gst->player.weapon_psys);
    update_psystem(gst, &gst->psystems[ENEMY_LVL0_WEAPON_PSYS]);
    update_psystem(gst, &gst->psystems[PLAYER_PRJ_ENVHIT_PSYS]);
    update_psystem(gst, &gst->psystems[ENEMY_PRJ_ENVHIT_PSYS]);
    update_psystem(gst, &gst->psystems[ENEMY_HIT_PSYS]);
}

#include <stdio.h>
#include <stdlib.h>

int compare(const void* p1, const void* p2) {
    const struct crithit_marker_t* a = (const struct crithit_marker_t*)p1;
    const struct crithit_marker_t* b = (const struct crithit_marker_t*)p2;
    
    // if a->dst >= b->dst. a goes before b.
    return (a->dst >= b->dst) ? -1 : 1;
}

static void _state_render_crithit_markers(struct state_t* gst) {
    struct crithit_marker_t* marker = NULL;

    struct crithit_marker_t sorted[MAX_RENDER_CRITHITS] = { 0 };
    int num_visible = 0;

    for(size_t i = 0; i < MAX_RENDER_CRITHITS; i++) {
        marker = &gst->crithit_markers[i];
        if(!marker->visible) {
            continue;
        }

        marker->dst = Vector3Distance(gst->player.position, marker->position);

        marker->lifetime += gst->dt;
        if(marker->lifetime >= gst->crithit_marker_maxlifetime) {
            marker->visible = 0;
            continue;
        }
        sorted[num_visible] = *marker;
        num_visible++;
    }

    if(num_visible == 0) {
        return;
    }

    // Sort by distance to fix alpha blending.
    qsort(sorted, num_visible, sizeof *sorted, compare);

    for(size_t i = 0; i < num_visible; i++) {
        struct crithit_marker_t* m = &sorted[i];

        // Interpolate scale and color alpha.

        float t = normalize(m->lifetime, 0, gst->crithit_marker_maxlifetime);
        float scale = lerp(t, 3.0, 0.0);
        float alpha = lerp(t, 255, 0.0);
   



        DrawBillboard(gst->player.cam, 
                gst->textures[CRITICALHIT_TEXID], m->position, scale, 
                (Color){ 180+ sin(gst->time*30)*50, 100, 25, alpha });
    }
}

void state_render_environment(struct state_t* gst) {


    // Render 3D stuff into texture and post process it later.
    BeginTextureMode(gst->env_render_target);
    ClearBackground((Color){(0.3) * 255, (0.15) * 255, (0.15) * 255, 255 });
    BeginMode3D(gst->player.cam);
    {

        // Render debug info if needed.
        if(gst->debug) {
            for(size_t i = 0; i < gst->num_enemies; i++) {
                struct enemy_t* ent = &gst->enemies[i];
      
                if(gst->debug) {          
                    DrawBoundingBox(get_enemy_boundingbox(ent), RED);
                }
            }

            DrawBoundingBox(get_player_boundingbox(&gst->player), GREEN);
        }


        BeginShaderMode(gst->shaders[DEFAULT_SHADER]);


        DrawCube((Vector3){ 20.0, 3.0, -10.0 }, 3.0, 3.0, 3.0, (Color){ 30, 30, 30, 255});

        // Terrain.
        render_terrain(gst, &gst->terrain);


        
        // Enemies.
        for(size_t i = 0; i < gst->num_enemies; i++) {
            struct enemy_t* ent = &gst->enemies[i];
            render_enemy(gst, ent);
        }

        player_render(gst, &gst->player);
        render_psystem(gst, &gst->player.weapon_psys, gst->player.weapon.color);
        render_psystem(gst, &gst->psystems[PLAYER_PRJ_ENVHIT_PSYS], gst->player.weapon.color);
        render_psystem(gst, &gst->psystems[ENEMY_PRJ_ENVHIT_PSYS], ENEMY_WEAPON_COLOR);
        render_psystem(gst, &gst->psystems[ENEMY_LVL0_WEAPON_PSYS], ENEMY_WEAPON_COLOR);
        render_psystem(gst, &gst->psystems[ENEMY_HIT_PSYS], (Color){ 255, 160, 20});
    

        EndShaderMode();
        
        _state_render_crithit_markers(gst);

    }
    EndMode3D();
    EndTextureMode();
    EndShaderMode();



    // Get bloom treshold texture.

    BeginTextureMode(gst->bloomtreshold_target);
    ClearBackground((Color){ 0,0,0, 255 });
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

void state_setup_all_shaders(struct state_t* gst) {

    const float fog_density = 0.006;
    

    // --- Setup Default Shader ---
    {
  
        Shader* shader = &gst->shaders[DEFAULT_SHADER];


        load_shader(
                "res/shaders/default.vs",
                "res/shaders/default.fs", shader);
        
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


    // --- Setup PRJ_ENVHIT_PSYS_SHADER ---
    {
        Shader* shader = &gst->shaders[PRJ_ENVHIT_PSYS_SHADER];
        load_shader(
                "res/shaders/particle_core.vs",
                "res/shaders/prj_envhit_psys.fs", shader);
       
        shader->locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(*shader, "mvp");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
    }
 

    // --- Setup PLAYER_WEAPON_PSYS_SHADER ---
    {
        Shader* shader = &gst->shaders[BASIC_WEAPON_PSYS_SHADER];
        load_shader(
                "res/shaders/particle_core.vs",
                "res/shaders/basic_weapon_psys.fs", shader);
       
        shader->locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(*shader, "mvp");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
    }
 


    // --- Setup Bloom Treshold Shader ---
    {
        Shader* shader = &gst->shaders[BLOOM_TRESHOLD_SHADER];
        *shader = LoadShader(
            0 /* use raylibs default vertex shader */, 
            "res/shaders/bloom_treshold.fs"
        );
    }

}

void state_create_enemy_weapons(struct state_t* gst) {

   
    // Player's weapon.
    gst->player.weapon = (struct weapon_t) {
        .id = PLAYER_WEAPON_ID,
        .accuracy = 7.85,
        .damage = 10.0,
        .critical_chance = 80,
        .critical_mult = 3.0,
        .prj_speed = 200.0,
        .prj_max_lifetime = 2.0,
        .prj_hitbox_size = (Vector3) { 1.0, 1.0, 1.0 },
        .color = (Color) { 20, 255, 150, 255 },
        .overheat_temp = 100.0,
        .heat_increase = 2.0,
        .cooling_level = 20.0
    };
 

    // Enemy lvl0 weapon.
    gst->enemy_weapons[ENEMY_LVL0_WEAPON] = (struct weapon_t) {
        .id = ENEMY_WEAPON_ID,
        .accuracy = 8.0,
        .damage = 5.0,
        .critical_chance = 25,
        .critical_mult = 3.0,
        .prj_speed = 90.0,
        .prj_max_lifetime = 5.0,
        .prj_hitbox_size = (Vector3) { 1.0, 1.0, 1.0 },
        .color = ENEMY_WEAPON_COLOR,
        .overheat_temp = -1,
    };

}

void state_create_psystems(struct state_t* gst) {

    // Create PLAYER_WEAPON_PSYS
    { // (projectile particle system for player)
        struct psystem_t* psystem = &gst->player.weapon_psys;
        create_psystem(
                gst,
                PSYS_GROUPID_PLAYER,
                psystem,
                32,
                weapon_psys_prj_update,
                weapon_psys_prj_init,
                BASIC_WEAPON_PSYS_SHADER
                );

        psystem->particle_mesh = GenMeshSphere(0.5, 16, 16);
        psystem->userptr = &gst->player.weapon;
    }

    // Create PLAYER_PRJ_ENVHIT_PSYS.
    { // (when player's projectile hits environment)
        struct psystem_t* psystem = &gst->psystems[PLAYER_PRJ_ENVHIT_PSYS];
        create_psystem(
                gst,
                PSYS_GROUPID_PLAYER,
                psystem,
                64,
                prj_envhit_psys_update,
                prj_envhit_psys_init,
                PRJ_ENVHIT_PSYS_SHADER
                );

        psystem->particle_mesh = GenMeshSphere(0.5, 32, 32);
        psystem->userptr = &gst->player.weapon;
    }



    // Create ENEMY_LVL0_WEAPON_PSYS.
    { // (projectile particle system for enemy lvl 0)
        struct psystem_t* psystem = &gst->psystems[ENEMY_LVL0_WEAPON_PSYS];
        create_psystem(
                gst,
                PSYS_GROUPID_ENEMY,
                psystem,
                32,
                weapon_psys_prj_update,
                weapon_psys_prj_init,
                BASIC_WEAPON_PSYS_SHADER
                );

        psystem->particle_mesh = GenMeshSphere(0.5, 16, 16);
        psystem->userptr = &gst->enemy_weapons[ENEMY_LVL0_WEAPON];
    }

    // Create ENEMY_PRJ_ENVHIT_PSYS.
    { // (when enemies projectile hits environment)
        struct psystem_t* psystem = &gst->psystems[ENEMY_PRJ_ENVHIT_PSYS];
        create_psystem(
                gst,
                PSYS_GROUPID_ENEMY,
                psystem,
                64,
                prj_envhit_psys_update,
                prj_envhit_psys_init,
                PRJ_ENVHIT_PSYS_SHADER
                );

        psystem->particle_mesh = GenMeshSphere(0.5, 32, 32);
        psystem->userptr = &gst->player.weapon;
    }


    // Create ENEMY_HIT_PSYS.
    { // (when enemy gets hit)
        struct psystem_t* psystem = &gst->psystems[ENEMY_HIT_PSYS];
        create_psystem(
                gst,
                PSYS_GROUPID_ENV,
                psystem,
                64,
                enemy_hit_psys_update,
                enemy_hit_psys_init,
                PRJ_ENVHIT_PSYS_SHADER
                );

        psystem->particle_mesh = GenMeshSphere(0.5, 32, 32);
        psystem->userptr = &gst->player.weapon;
    }


}

void state_delete_psystems(struct state_t* gst) {
    delete_psystem(&gst->psystems[PLAYER_PRJ_ENVHIT_PSYS]);
    delete_psystem(&gst->psystems[ENEMY_PRJ_ENVHIT_PSYS]);
    delete_psystem(&gst->psystems[ENEMY_LVL0_WEAPON_PSYS]);
    delete_psystem(&gst->psystems[ENEMY_HIT_PSYS]);
}


void state_add_crithit_marker(struct state_t* gst, Vector3 position) {
    struct crithit_marker_t* marker = &gst->crithit_markers[gst->num_crithit_markers];

    marker->visible = 1;
    marker->lifetime = 0.0;
    marker->position = position;

    const float r = 7.0;
    marker->position.x += RSEEDRANDOMF(-r, r);
    marker->position.z += RSEEDRANDOMF(-r, r);
    marker->position.y += RSEEDRANDOMF(r, r*2)*0.2;

    gst->num_crithit_markers++;
    if(gst->num_crithit_markers >= MAX_RENDER_CRITHITS) {
        gst->num_crithit_markers = 0;
    }
}



