#ifndef SHADER_UTIL_H
#define SHADER_UTIL_H

#include <raylib.h>

struct state_t;

typedef short uloc_t;


#define U_TIME 0
#define U_CAMPOS 1
#define U_SCREEN_SIZE 2
#define U_WATERLEVEL 3
#define U_GBUFPOS_TEX 4
#define U_GBUFNORM_TEX 5
#define U_GBUFDIFSPEC_TEX 6
#define U_GBUFDEPTH_TEX 7
#define U_SSAO_NOISE_TEX 8
#define U_CAMVIEW_MATRIX 9
#define U_CAMPROJ_MATRIX 10
#define U_GUNFX_COLOR 11
#define U_SSAO_ENABLED 12
#define U_ANYGUI_OPEN 13

#define U_NOTFOUND -1

#define MAX_UNIFORM_LOCS 64

// gst->shaderutil[shader_index].ulocs[U_TIME]

// This will take a littlebit unused memory but will make programming with shaders alot easier
// by not having to setup the uniform locations by yourself.
struct shaderutil_t {
    uloc_t ulocs[MAX_UNIFORM_LOCS];
};


void init_shaderutil(struct state_t* gst);


// If shader uniform location in 'shaderutil_t ulocs' array is 'U_NOTFOUND'.
// it will try to get it from the shader by name (static names are defined in 'shader_util.c')

void shader_setu_float (struct state_t* gst, int shader_index, int shader_u, float* v);
void shader_setu_int   (struct state_t* gst, int shader_index, int shader_u, int* v);
void shader_setu_vec2  (struct state_t* gst, int shader_index, int shader_u, Vector2* v);
void shader_setu_vec3  (struct state_t* gst, int shader_index, int shader_u, Vector3* v);
void shader_setu_vec4  (struct state_t* gst, int shader_index, int shader_u, Vector4* v);
void shader_setu_sampler (struct state_t* gst, int shader_index, int shader_u, int texid);
void shader_setu_matrix  (struct state_t* gst, int shader_index, int shader_u, Matrix m);
void shader_setu_color   (struct state_t* gst, int shader_index, int shader_u, Color* c);

#endif
