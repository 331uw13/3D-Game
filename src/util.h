#ifndef UTIL_H
#define UTIL_H

#include <raylib.h>
#include <raymath.h>

struct state_t;


void rainbow_palette(float t, unsigned char* red, unsigned char* grn, unsigned char* blu);

// Raylib does have MatrixTranslate function
// but it clears rotation and scale.
// this only changes the poisition
void matrix_addtransl(Matrix* m, float x, float y, float z);


int setup_3Dmodel(struct state_t* gst, Model* model, const char* model_filepath, Vector3 init_pos);

float normalize (float t, float min, float max);
float lerp      (float t, float min, float max);
float map       (float t, float src_min, float src_max, float dst_min, float dst_max);

#endif
