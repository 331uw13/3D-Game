#ifndef TERRAIN_CHUNK_H
#define TERRAIN_CHUNK_H

#include <raylib.h>

#include "biome.h"

struct state_t;
struct terrain_t;

// "Foliage" index

#define TF_COMFY_TREE_0 0
#define TF_COMFY_TREE_1 1
#define TF_COMFY_ROCK_0 2
#define TF_COMFY_MUSHROOM_0 3

#define TF_HAZY_TREE_0 4
#define TF_HAZY_ROCK_0 5
// ...

#define MAX_FOLIAGE_TYPES 6



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


struct chunk_area_t {
    float x_min;
    float x_max;
    float z_min;
    float z_max;
};

struct chunk_t {
    size_t   index; // Index in 'terrain.chunks' array.
    Mesh     mesh;
    Vector3  position;
    Vector3  center_pos;
    float    dst2player;
    struct chunk_foliage_data_t foliage_data[MAX_FOLIAGE_TYPES];
    struct biome_t biome;
    size_t grass_baseindex; // Chunks first grass blade index.

    struct chunk_area_t area;

    /*
    // Texture for grass force vectors.
    // When chunk grass is going to be rendered
    // force vectors are updated into force vector ubo
    // the shaader then writes the vector as color into this texture.
    // Then these textures are used in the compute shader
    // to calculate the extra rotations for the grass blade.
    // This will disable the need for big and expensive loop in the compute shader.
    RenderTexture2D forcetex;
    */
};

void load_foliage_models(struct state_t* gst, struct terrain_t* terrain);
void load_chunk_foliage(struct state_t* gst, struct terrain_t* terrain, struct chunk_t* chunk);
void decide_chunk_biome(struct state_t* gst, struct terrain_t* terrain, struct chunk_t* chunk);
void delete_chunk(struct chunk_t* chunk);
void load_chunk(
        struct state_t* gst,
        struct terrain_t* terrain,
        struct chunk_t* chunk,
        int chunk_x,
        int chunk_z,
        int chunk_triangle_count
        );
struct chunk_t* find_chunk(struct state_t* gst, Vector3 position);


void render_chunk_grass(
        struct state_t* gst,
        struct terrain_t* terrain,
        struct chunk_t* chunk,
        Matrix* mvp,
        int render_pass
        );

// These can be used for debug if needed.
void render_chunk_borders(struct state_t* gst, struct chunk_t* chunk, Color color);
void render_chunk_borders2x(struct state_t* gst, struct chunk_t* chunk, Color color);

#endif
