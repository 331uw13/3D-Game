#ifndef TERRAIN_H
#define TERRAIN_H

#include <stddef.h>
#include <raylib.h>

#include "perlin_noise.h"
#include "typedefs.h"

struct state_t;


#define RENDER_DISTANCE 1800
#define CHUNK_SIZE 64


#define TREE_TYPE0_MAX_PERCHUNK 30
#define ROCK_TYPE0_MAX_PERCHUNK 10

struct heightmap_t {
    float*  data;
    size_t  total_size; // equals to (size * size)

    // NOTE: this is size for width AND depth
    u32 size;
};

struct triangle2x_t { // holds 2 triangles (1 quad).
    Vector3 a0;
    Vector3 a1;
    Vector3 a2;    

    Vector3 b0;
    Vector3 b1;
    Vector3 b2;
};


struct foliage_matrices_t {
    Matrix tree_type0[TREE_TYPE0_MAX_PERCHUNK];
    size_t num_tree_type0;
    
    Matrix rock_type0[ROCK_TYPE0_MAX_PERCHUNK];
    size_t num_rock_type0;
};

struct foliage_models_t {
    Model tree_type0;
    Model rock_type0;
};

struct chunk_t {
    Mesh     mesh;
    Vector3  position;
    Vector3  center_pos;
    float    dst2player;

    struct foliage_matrices_t foliage_matrices;
};

struct terrain_t {
    Mesh      mesh; // The whole terrain mesh in one.
    Material  material;
    Matrix    transform;
    struct heightmap_t heightmap;

    struct chunk_t* chunks;
    int    chunk_size;
    size_t num_chunks;

    int num_visible_chunks;
    struct foliage_models_t foliage_models;
 

    float highest_point;
    float scaling;

    // triangles saved but in order to get triangle at xz location efficiently.
    struct triangle2x_t* triangle_lookup;
};


// More optimized way to raycast the terrain instead of raycasting on the whole terrain mesh.
// it uses triangle lookup table.
RayCollision raycast_terrain(struct terrain_t* terrain, float x, float z);

Matrix get_rotation_to_surface(
        struct terrain_t* terrain,
        float x, float z,
        float* hit_y // report back the surface y position ray hit?
        );


void generate_terrain(
        struct state_t* gst,
        struct terrain_t* terrain,
        u32    terrain_size,
        float  terrain_scaling,
        float  amplitude,
        float  frequency,
        int    octaves,
        int    seed
        );

//void generate_terrain_foliage(struct state_t* gst, struct terrain_t* terrain);

void delete_terrain         (struct terrain_t* terrain);

void render_terrain(struct state_t* gst, struct terrain_t* terrain, int shader_id);




#endif
