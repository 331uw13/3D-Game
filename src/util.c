#include <math.h>
#include <stdio.h>

#include "util.h"
#include "state.h"

#define _2PI 6.28318


void rainbow_palette(float t, unsigned char* red, unsigned char* grn, unsigned char* blu) {
    // for more information about this check out:
    // https://iquilezles.org/articles/palettes/
    *red = (unsigned char)((0.5+0.5 * cos(_2PI * t)) * 255.0);
    *grn = (unsigned char)((0.5+0.5 * cos(_2PI * (t+0.33))) * 255.0);
    *blu = (unsigned char)((0.5+0.5 * cos(_2PI * (t+0.67))) * 255.0);

}


void matrix_addtransl(Matrix* m, float x, float y, float z) {
    m->m12 = x;
    m->m13 = y;
    m->m14 = z;
}

void add_movement_vec3(Vector3* v1, Vector3 dir, float f) {
    dir.x *= f;
    dir.y *= f;
    dir.z *= f;
    *v1 = Vector3Add(*v1, dir);
}


int setup_3Dmodel(struct state_t* gst, Model* model, const char* model_filepath, int texture_id, Vector3 init_pos) {
    int ok = 0;

    if(!FileExists(model_filepath)) {
        fprintf(stderr, "\033[31m%s: '%s' doesnt exist.\033[0m\n",
                __func__, model_filepath);
        goto error;
    }

    *model = LoadModel(model_filepath);
    model->materials[0].shader = gst->shaders[DEFAULT_SHADER];
    model->transform = MatrixTranslate(init_pos.x, init_pos.y, init_pos.z);

    if(texture_id >= 0) {
        model->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = gst->tex[texture_id];
    }

    ok = 1;

error:

    return ok;
}

float angle_xz(Vector3 a, Vector3 b) {
    Vector3 diff = Vector3Subtract(a, b);
    return -(atan2(diff.z, diff.x) + M_PI);
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


