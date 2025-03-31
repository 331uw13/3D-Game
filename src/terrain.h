#ifndef TERRAIN_H
#define TERRAIN_H

#include <stddef.h>
#include <raylib.h>

#include "perlin_noise.h"
#include "typedefs.h"

struct state_t;


#define CHUNK_SIZE 64

#define WATER_INITIAL_YLEVEL -230 // NOTE: This must be same as in 'res/shaders/default.fs'


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


#define TF_TREE_TYPE0 0
#define TF_TREE_TYPE1 1
#define TF_ROCK_TYPE0 2
#define MAX_FOLIAGE_TYPES 3


// This data is not directly being used to render foliages from.
//   When chunks are being generated
//   this array will be filled with their transformation matrix
//   then they are copied to 'foliage_rdata' and rendered all at once(per type)
//   ^ this will reduce the draw calls.
struct chunk_foliage_data_t {
    Matrix* matrices;         
    size_t  matrices_size;   // How many elements was allocated for matrix array?
    size_t  num_foliage;     // Chunk may not generate the absolute max number of foliage.
};

// "Foliage render data"
// Matrices from all visible chunks to player.
struct foliage_rdata_t {
    Matrix* matrices;      
    size_t  matrices_size;   // How many elemets was allocated for matrix array?
    size_t  num_render;      // How many to render?
    size_t  next_index;      // Keep track of index where to copy.
};

struct chunk_t {
    Mesh     mesh;
    Vector3  position;
    Vector3  center_pos;
    float    dst2player;

    struct chunk_foliage_data_t foliage_data[MAX_FOLIAGE_TYPES];
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

    size_t                 foliage_max_perchunk  [MAX_FOLIAGE_TYPES];
    Model                  foliage_models [MAX_FOLIAGE_TYPES];
    struct foliage_rdata_t foliage_rdata  [MAX_FOLIAGE_TYPES];

    float highest_point;
    float lowest_point;
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


void delete_terrain(struct terrain_t* terrain);
void render_terrain(struct state_t* gst, struct terrain_t* terrain);




#endif
