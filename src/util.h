#ifndef UTIL_H
#define UTIL_H

#include <raylib.h>
#include <raymath.h>

struct state_t;

#define RANDOMGEN_MAX 0x7FFF

void rainbow_palette(float t, unsigned char* red, unsigned char* grn, unsigned char* blu);

// multiply direction by factor(f) and add it to v1
void add_movement_vec3(Vector3* v1, Vector3 dir, float f);

//int setup_3Dmodel(struct state_t* gst, Model* model, const char* model_filepath, int texture_id, Vector3 init_pos);

float angle_xz(Vector3 a, Vector3 b);
Vector3 vec3mult_v(Vector3 a, float v);
Vector3 vec3set_mag(Vector3 a, float nm);
float normalize (float t, float min, float max);
float lerp      (float t, float min, float max);
float map       (float t, float src_min, float src_max, float dst_min, float dst_max);

int    randomgen (int* seed);
int    randomi(int* seed, int min, int max);
float  randomf(int* seed, float min, float max);

Color color_lerp(float t, Color a, Color b);

#define RSEEDRANDOMF(min, max) randomf(&gst->rseed, min, max)
#define CLAMP(v, min, max) ((v < min) ? min : (v > max) ? max : v)
#define MISSING_PSYSUSERPTR fprintf(stderr, "\033[31m(ERROR) '%s': Missing psystem 'userptr'\033[0m\n", __func__)

#endif
