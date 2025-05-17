#ifndef FRACTALGEN_H
#define FRACTALGEN_H

#include <raylib.h>


struct state_t;


struct fractal_t {
    Mesh mesh;
    Material material;
    Matrix transform;
};


void render_fractal_model(struct fractal_t* fmodel);
void delete_fractal_model(struct fractal_t* fmodel);


void fractalgen_tree(
        struct state_t* gst,
        struct fractal_t* fmodel,
        Vector3 rotation_weights,
        float start_height,
        float dampen_height,
        float start_cube_scale,
        float dampen_cube_scale,
        Color start_color,
        Color end_color
        );






#endif
