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


struct chunk_t {
    size_t   index; // Index in 'terrain.chunks' array.
    Mesh     mesh;
    Vector3  position;
    Vector3  center_pos;
    float    dst2player;
    struct chunk_foliage_data_t foliage_data[MAX_FOLIAGE_TYPES];
    struct biome_t biome;
};


void load_foliage_models(struct state_t* gst, struct terrain_t* terrain);
void load_chunk_foliage(struct state_t* gst, struct terrain_t* terrain, struct chunk_t* chunk);
void decide_chunk_biome(struct state_t* gst, struct terrain_t* terrain, struct chunk_t* chunk);


#endif
