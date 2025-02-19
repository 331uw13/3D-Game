#ifndef TERRAIN_H
#define TERRAIN_H

#include <raylib.h>

#include "perlin_noise.h"

struct state_t;


#define HEIGHTMAP_SIZE_X 300
#define HEIGHTMAP_SIZE_Z 300
#define HEIGHTMAP_SIZE_XZ (HEIGHTMAP_SIZE_X * HEIGHTMAP_SIZE_Z)


struct heightmap_t {
    float data[HEIGHTMAP_SIZE_XZ];
    int   size_x;
    int   size_z;
    int   ready;
};


struct triangle2x_t {
    Vector3 a0;
    Vector3 a1;
    Vector3 a2;
    
    Vector3 b0;
    Vector3 b1;
    Vector3 b2;
};

struct terrain_t {
    Mesh      mesh;
    Material  material;
    Matrix    transform;
    int  mesh_generated;
    struct heightmap_t heightmap;
 
    float highest_point;
    float xz_scale;

    // triangles saved but in order to get triangle at xz location efficiently.
    struct triangle2x_t* triangle_lookup;
};



//size_t get_heightmap_index(struct terrain_t* terrain, float x, float z);
//float  get_heightmap_value(struct terrain_t* terrain, float x, float z);
float  get_smooth_heightmap_value(struct terrain_t* terrain, float x, float z);

void generate_heightmap(struct terrain_t* terrain);
void generate_terrain_mesh(struct state_t* gst, struct terrain_t* terrain);
void delete_terrain(struct terrain_t* terrain);

void render_terrain(struct state_t* gst, struct terrain_t* terrain);




#endif
