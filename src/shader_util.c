#include <stdio.h>
#include "state/state.h"

#include <rlgl.h>


#define DISABLE_ERROR_MSG 1

// NOTE: This must match order with '#define U_...' in 'shader_util.h'
static const char* g_shader_uniform_names[MAX_UNIFORM_LOCS] = {
    "u_time\0",
    "u_campos\0",
    "u_screen_size\0",
    "u_waterlevel\0",
    "u_gbuf_pos_tex\0",
    "u_gbuf_norm_tex\0",
    "u_gbuf_difspec_tex\0",
    "u_gbuf_depth_tex\0",
    "u_ssao_noise_tex\0",
    "u_camview_matrix\0",
    "u_camproj_matrix\0",
    "u_gunfx_color\0",
    "u_ssao_enabled\0",
    "u_anygui_open\0",
    "u_bloomtresh_tex\0",
    "u_render_dist\0",
    "u_defnoise_tex\0",
    "u_ssao_kernel_samples\0",
    "u_shadow_bias\0",
    "u_shadow_res\0",
    "u_wind_strength\0",
    "u_wind_dir\0",
    "u_sun_color\0",
    "u_ground_pass\0",
    "u_terrain_lowest\0",
    "u_terrain_highest\0"
};


static int get_uloc(struct state_t* gst, int shader_index, int shader_u) {
    int res = U_NOTFOUND;

    if((shader_u < 0) || (shader_u >= MAX_UNIFORM_LOCS)) {
        fprintf(stderr, "\033[31m(ERROR) '%s': 'shader_u' out of bounds for shader index %i.\033[0m\n",
                __func__, shader_index);
        goto error;
    }

    uloc_t* ulocptr = &gst->shader_u[shader_index].ulocs[shader_u];

    if(*ulocptr == U_NOTFOUND) {
        int loc = GetShaderLocation(gst->shaders[shader_index], g_shader_uniform_names[shader_u]);
        if(loc < 0) {
            if(!DISABLE_ERROR_MSG) {
                fprintf(stderr, "\033[31m(ERROR) '%s': Uniform with name '%s' not found for shader index %i, loc=%i\033[0m\n"
                    "\033[90m(Note: shader compiler might optimize out unused uniforms!)\033[0m\n",
                    __func__, g_shader_uniform_names[shader_u], shader_index, loc);
            }
            goto error;
        }
       
        printf("Shader uniform found: '%s' (%i) shader_index=%i\n",
                g_shader_uniform_names[shader_u], loc, shader_index);
        *ulocptr = (uloc_t)loc;
    }

    res = (int)*ulocptr;

error:
    return res;
}

void init_shaderutil(struct state_t* gst) {
    for(size_t s_i = 0; s_i < MAX_SHADERS; s_i++) {
        for(size_t i = 0; i < MAX_UNIFORM_LOCS; i++) {
            gst->shader_u[s_i].ulocs[i] = U_NOTFOUND;
        }
    }
}


void shader_setu_float(struct state_t* gst, int shader_index, int shader_u, float* v) {
    int loc = get_uloc(gst, shader_index, shader_u);
    if(loc == U_NOTFOUND) {
        return;
    }

    SetShaderValueV(gst->shaders[shader_index], loc, v, SHADER_UNIFORM_FLOAT, 1);
}

void shader_setu_int(struct state_t* gst, int shader_index, int shader_u, int* v) {
    int loc = get_uloc(gst, shader_index, shader_u);
    if(loc == U_NOTFOUND) {
        return;
    }

    SetShaderValueV(gst->shaders[shader_index], loc, v, SHADER_UNIFORM_INT, 1);
}


void shader_setu_vec2(struct state_t* gst, int shader_index, int shader_u, Vector2* v) {
    int loc = get_uloc(gst, shader_index, shader_u);
    if(loc == U_NOTFOUND) {
        return;
    }

    SetShaderValueV(gst->shaders[shader_index], loc, v, SHADER_UNIFORM_VEC2, 1);
}

void shader_setu_vec3(struct state_t* gst, int shader_index, int shader_u, Vector3* v) {
    int loc = get_uloc(gst, shader_index, shader_u);
    if(loc == U_NOTFOUND) {
        return;
    }

    SetShaderValueV(gst->shaders[shader_index], loc, v, SHADER_UNIFORM_VEC3, 1);
}

void shader_setu_vec4(struct state_t* gst, int shader_index, int shader_u, Vector4* v) {
    int loc = get_uloc(gst, shader_index, shader_u);
    if(loc == U_NOTFOUND) {
        return;
    }

    SetShaderValueV(gst->shaders[shader_index], loc, v, SHADER_UNIFORM_VEC4, 1);
}

void shader_setu_ivec4 (struct state_t* gst, int shader_index, int shader_u, struct vec4int_t* v) {
    int loc = get_uloc(gst, shader_index, shader_u);
    if(loc == U_NOTFOUND) {
        return;
    }

    SetShaderValueV(gst->shaders[shader_index], loc, v, SHADER_UNIFORM_IVEC4, 1);
}

void shader_setu_sampler(struct state_t* gst, int shader_index, int shader_u, int texid) {
    int loc = get_uloc(gst, shader_index, shader_u);
    if(loc == U_NOTFOUND) {
        return;
    }

    rlEnableShader(gst->shaders[shader_index].id);
    rlSetUniformSampler(loc, texid);
}

void shader_setu_matrix(struct state_t* gst, int shader_index, int shader_u, Matrix m) {
    int loc = get_uloc(gst, shader_index, shader_u);
    if(loc == U_NOTFOUND) {
        return;
    }

    SetShaderValueMatrix(gst->shaders[shader_index], loc, m);
}

void shader_setu_color(struct state_t* gst, int shader_index, int shader_u, Color* c) {
    
    Vector4 v4f = (Vector4) {
        (float)c->r / 255.0,
        (float)c->g / 255.0,
        (float)c->b / 255.0,
        (float)c->a / 255.0
    };

    shader_setu_vec4(gst, shader_index, shader_u, &v4f);

}

