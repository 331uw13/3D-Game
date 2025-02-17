#ifndef TERRAIN_H
#define TERRAIN_H

#include <raylib.h>

#include "perlin_noise.h"

struct state_t;


#define HEIGHTMAP_SIZE_X 100
#define HEIGHTMAP_SIZE_Z 100
#define HEIGHTMAP_SIZE_XZ (HEIGHTMAP_SIZE_X * HEIGHTMAP_SIZE_Z)


struct heightmap_t {
    float data[HEIGHTMAP_SIZE_XZ];
    int   size_x;
    int   size_z;
    int   ready;
};



struct terrain_t {
    Mesh      mesh;
    Material  material;
    Matrix    transform;
    int  mesh_generated;
    struct heightmap_t heightmap;

    float highest_point;
    float xz_scale;
};



size_t get_heightmap_index(struct terrain_t* terrain, float x, float z);
float get_heightmap_value(struct terrain_t* terrain, float x, float z);

void generate_heightmap(struct terrain_t* terrain);
void generate_terrain_mesh(struct state_t* gst, struct terrain_t* terrain);
void delete_terrain(struct terrain_t* terrain);

void render_terrain(struct state_t* gst, struct terrain_t* terrain);




#endif
