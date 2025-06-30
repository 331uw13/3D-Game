#ifndef FRACTALGEN_H
#define FRACTALGEN_H

#include <raylib.h>


// The fractal has a special function when player interacts with it.
// it is just called 'fractal' in the code
//  because i didnt know any good name for it and it is a fractal type tree..


//#define FRACTAL_TYPE_ENERGY_SRC 0  // Weapons use this as their ammo.
//#define FRACTAL_TYPE_HEALTH_SRC 1
//#define MAX_FRACTAL_TYPES 2

#define MAX_BERRIES_PER_FRACTAL 8
#define MAX_BERRY_LEVEL 1.0
#define MIN_BERRY_LEVEL 0.25

struct state_t;


struct berry_t {
    Vector3  position;
    float    level;
};

struct fractal_t {
    Mesh      mesh;
    Material  material;
    Matrix    transform;
    Vector3   scale; // Needed for berries.
    
    struct berry_t berries[MAX_BERRIES_PER_FRACTAL];
    Color berry_color;
    int num_berries;

    int liquid_type; // See 'src/items/lqcontainer.h' for available types.
};


void delete_fractal_model(struct fractal_t* fmodel);


void fractalgen_tree(
        struct state_t* gst,
        struct fractal_t* fmodel,
        int depth,
        Vector3 rotation_weights,
        float start_height,
        float dampen_height,
        float start_cube_scale,
        float dampen_cube_scale,
        Color start_color,
        Color end_color
        );






#endif
