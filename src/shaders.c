#include <stdlib.h>

#include "shaders.h"

#include "state.h"
#include "glsl_preproc.h"


// Load shaders but preprocess fragment shader.
int load_shader(const char* vs_filename, const char* fs_filename, Shader* shader) {
    int result = 0;
    
    struct file_t fragment_file  = { .data = NULL, .size = 0 };
    struct file_t vertex_file    = { .data = NULL, .size = 0 };



    // Errors are reported from functions.

    if(!read_file(&vertex_file, vs_filename)) {
        goto error;
    }
    if(!read_file(&fragment_file, fs_filename)) {
        goto error_and_close;
    }

    
    size_t sizeout = 0;
    char* fs_code = preproc_glsl(&fragment_file, &sizeout);

    *shader = LoadShaderFromMemory(vertex_file.data, fs_code);

    if(fs_code) {
        free(fs_code);
    }

    result = 1;

error_and_close:
    close_file(&vertex_file);
    close_file(&fragment_file);

error:
    return result;
}


void setup_all_shaders(struct state_t* gst) {

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

 
    // --- Setup Projectiles (Particle System) shader ---
    {
        Shader* shader = &gst->shaders[PROJECTILES_PSYSTEM_SHADER];
        load_shader(
                "res/shaders/particle_core.vs",
                "res/shaders/projectiles_psystem.fs", shader);
       
        shader->locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(*shader, "mvp");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
        
        gst->fs_unilocs[PROJECTILES_PSYSTEM_COLOR_FS_UNILOC] = GetShaderLocation(*shader, "psystem_color");
       
    }
 

    // --- Setup Projectiles (Particle System) WHEN HIT shader ---
    {
        Shader* shader = &gst->shaders[PROJECTILES_ENVHIT_SHADER];
        load_shader(
                "res/shaders/particle_core.vs",
                "res/shaders/projectiles_hit.fs", shader);
       
        shader->locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(*shader, "mvp");
        shader->locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(*shader, "viewPos");
        shader->locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(*shader, "instanceTransform");
        
        gst->fs_unilocs[PROJECTILES_ENVHIT_COLOR_FS_UNILOC] = GetShaderLocation(*shader, "psystem_color");
        gst->fs_unilocs[PROJECTILES_ENVHIT_TIME_FS_UNILOC] = GetShaderLocation(*shader, "time");
       
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


