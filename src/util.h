#ifndef UTIL_H
#define UTIL_H

#include <raylib.h>
#include <raymath.h>

struct state_t;

#define RANDOMGEN_MAX 0x7FFF

void rainbow_palette(float t, unsigned char* red, unsigned char* grn, unsigned char* blu);

// Raylib does have MatrixTranslate function
// but it clears rotation and scale.
// this only changes the poisition
void matrix_addtransl(Matrix* m, float x, float y, float z);

// multiply direction by factor(f) and add it to v1
void add_movement_vec3(Vector3* v1, Vector3 dir, float f);

int setup_3Dmodel(struct state_t* gst, Model* model, const char* model_filepath, int texture_id, Vector3 init_pos);

float angle_xz(Vector3 a, Vector3 b);
float normalize (float t, float min, float max);
float lerp      (float t, float min, float max);
float map       (float t, float src_min, float src_max, float dst_min, float dst_max);

int    randomgen (int* seed);
int    randomi(int* seed, int min, int max);
float  randomf(int* seed, float min, float max);

#define RSEEDRANDOMF(min, max) randomf(&gst->rseed, min, max)

#endif
