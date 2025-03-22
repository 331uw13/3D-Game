#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "state.h"
#include "glsl_preproc.h"

#define _2PI 6.28318


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



void rainbow_palette(float t, unsigned char* red, unsigned char* grn, unsigned char* blu) {
    // for more information about this check out:
    // https://iquilezles.org/articles/palettes/
    *red = (unsigned char)((0.5+0.5 * cos(_2PI * t)) * 255.0);
    *grn = (unsigned char)((0.5+0.5 * cos(_2PI * (t+0.33))) * 255.0);
    *blu = (unsigned char)((0.5+0.5 * cos(_2PI * (t+0.67))) * 255.0);

}



void add_movement_vec3(Vector3* v1, Vector3 dir, float f) {
    dir.x *= f;
    dir.y *= f;
    dir.z *= f;
    v1->x += dir.x * f;
    v1->y += dir.y * f;
    v1->z += dir.z * f;
}

Vector3 get_rotation_yz(Vector3 p1, Vector3 p2) {
    Vector3 d = Vector3Subtract(p2, p1);
    float dlen = Vector3Length(d);
    
    return (Vector3) {
        0.0, // Ignore X (roll)
        -(atan2(p1.z-p2.z, p1.x-p2.x)),
        acos(d.y / dlen) - 1.570795
    };
}

float angle_xz(Vector3 a, Vector3 b) {
    Vector3 diff = Vector3Subtract(a, b);
    return -(atan2(diff.z, diff.x) + M_PI);
}


Vector3 vec3mult_v(Vector3 a, float v) {
    return (Vector3) { a.x*v, a.y*v, a.z*v };
}

Vector3 vec3set_mag(Vector3 a, float nm) {
    float l = Vector3Length(a);
    return (Vector3) {
        a.x * nm / l,
        a.y * nm / l,
        a.z * nm / l
    };
}

float normalize(float t, float min, float max) {
    return (t - min) / (max - min);
}

float lerp(float t, float min, float max) {
    return (max - min) * t + min;
}

float map(float t, float src_min, float src_max, float dst_min, float dst_max) {
    return (t - src_min) * (dst_max - dst_min) / (src_max - src_min) + dst_min;
}

int randomgen (int* seed) {
    *seed = 0x343FD * *seed + 0x269EC3;
    return (*seed >> 16) & RANDOMGEN_MAX;
}

int randomi(int* seed, int min, int max) {
    return randomgen(seed) % (max - min) + min;
}

float randomf(int* seed, float min, float max) {
    return ((float)randomgen(seed) / ((float)RANDOMGEN_MAX / (max - min))) + min;
}

Color color_lerp(float t, Color a, Color b) {
    return (Color) {
        lerp(t, (float)a.r/255.0, (float)b.r/255.0) * 255,
        lerp(t, (float)a.g/255.0, (float)b.g/255.0) * 255,
        lerp(t, (float)a.b/255.0, (float)b.b/255.0) * 255,
        255
    };
}

float get_volume_dist(Vector3 player_pos, Vector3 sound_pos) {
    float dst = CLAMP(Vector3Distance(player_pos, sound_pos), 
            0.0, MAX_VOLUME_DIST);
    return CLAMP(1.0 - (dst / MAX_VOLUME_DIST), 0.0, 0.6);
}
