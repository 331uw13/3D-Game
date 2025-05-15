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


void fractalgen_tree(struct state_t* gst, struct fractal_t* fmodel);






#endif
