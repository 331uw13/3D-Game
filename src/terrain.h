#ifndef TERRAIN_H
#define TERRAIN_H

#include <stddef.h>
#include <raylib.h>

#include "perlin_noise.h"
#include "typedefs.h"

struct state_t;


#define RENDER_DISTANCE 3000
#define CHUNK_SIZE 64

#define WATER_INITIAL_YLEVEL -80 // NOTE: This must be same as in 'res/shaders/default.fs'

#define TREE_TYPE0_MAX_PERCHUNK 40
#define ROCK_TYPE0_MAX_PERCHUNK 10
#define CRYSTALS_MAX_PERCHUNK 1

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


// For each chunk.
struct foliage_matrices_t {
    Matrix tree_type0[TREE_TYPE0_MAX_PERCHUNK];
    size_t num_tree_type0;
    
    Matrix rock_type0[ROCK_TYPE0_MAX_PERCHUNK];
    size_t num_rock_type0;
    
    Matrix crystals[CRYSTALS_MAX_PERCHUNK];
    size_t num_crystals;
};



// All foliage matrices from chunks go here each frame 
// and they are rendered all at once.
struct render_foliage_matrices {
    Matrix* tree_type0;         // Matrices from all visible chunks.
    size_t  tree_type0_size;    // How many elements was allocated for matrix array.
    size_t  num_tree_type0;     // How many to render.

    Matrix* rock_type0;
    size_t  rock_type0_size;
    size_t  num_rock_type0;
    
    Matrix* crystals;
    size_t  crystals_size;
    size_t  num_crystals;
    
};

struct foliage_models_t {
    Model tree_type0;
    Model rock_type0;
    Model crystal;
};

struct chunk_t {
    Mesh     mesh;
    Vector3  position;
    Vector3  center_pos;
    float    dst2player;

    struct foliage_matrices_t foliage_matrices;
};

struct terrain_t {
    Material  material;
    Matrix    transform;
    struct heightmap_t heightmap;

    struct chunk_t* chunks;
    int    chunk_size;
    size_t num_chunks;
    int    num_max_visible_chunks;
    int    num_visible_chunks;

    struct foliage_models_t         foliage_models;
    struct render_foliage_matrices  rfmatrices;

    float highest_point;
    float scaling;

    float water_ylevel;
    Model waterplane;

    // Triangles saved but in order to get triangle at xz location efficiently.
    struct triangle2x_t* triangle_lookup;

    Vector3 valid_player_spawnpoint;
};


// More optimized way to raycast the terrain instead of raycasting on the whole terrain mesh.
// it uses triangle lookup table.
RayCollision raycast_terrain(struct terrain_t* terrain, float x, float z);

Matrix get_rotation_to_surface(struct terrain_t* terrain, float x, float z, RayCollision* ray_out);

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

void delete_terrain(struct terrain_t* terrain);

void render_terrain(
        struct state_t* gst,
        struct terrain_t* terrain,
        int terrain_shader_index,
        int foliage_shader_index
        );




#endif
