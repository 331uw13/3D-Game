#ifndef TERRAIN_H
#define TERRAIN_H

#include <stddef.h>
#include <raylib.h>

#include "perlin_noise.h"
#include "typedefs.h"
#include "chunk.h"

struct state_t;
struct fog_t;

#define CHUNK_SIZE 128

// NOTE: These distances must be adjusted if CHUNK_SIZE or terrain scale is changed.
// (TODO: Probably should calculate these in runtime or config file...)

//#define TERRAIN_LOW_RENDERDIST 3000 // Used by gbuffer passes and shadow cams.
//#define GRASS_MIN_RENDERDIST 6000.0 // TODO: This must be adjusted better (?)


// (CURRENTLY NOT USED)
#define WATER_INITIAL_YLEVEL -230


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

// "Foliage render data"
// Matrices from all visible chunks to player.
struct foliage_rdata_t {
    Matrix* matrices;      
    size_t  matrices_size;   // How many elemets was allocated for matrix array?
    size_t  num_render;      // How many to render?
    size_t  next_index;      // Keep track of index where to copy.
    int     render_backface;
};

#define GRASSDATA_STRUCT_SIZE (4*4/*position*/ + 4*4/*settings*/ + 48/*rotation (mat3x4)*/ + 4*4)

#define MAX_GRASS_CHUNKS 32

struct terrain_t {
    int seed;
    Material  material;  // <-TODO: Remove this
    Matrix    transform; // <-TODO: Remove this
    struct heightmap_t heightmap;

    struct chunk_t* chunks;
    int    chunk_size;
    size_t num_chunks;
    int    num_max_visible_chunks;
    int    num_visible_chunks;
    int    num_rendered_grass;

    Model  grass_model;
    Model  grass_model_lowres;
    size_t grass_instances_perchunk;

    struct chunk_t* grass_chunks[MAX_GRASS_CHUNKS];
    size_t num_grass_chunks;

    struct biome_t biomedata[MAX_BIOME_TYPES]; //<- Initialized from 'biome.c' 'setup_biomes()'
    Material biome_materials[MAX_BIOME_TYPES];
    Vector2  biome_ylevels[MAX_BIOME_TYPES]; // X(where the biome starts.) Y(where the biome ends.)
    
    // Area at biome edges
    float biomeshift_area; // NOTE: This value is set from 'biome.c' 'setup_biomes()'

    // Information about specific type of "foliage"
    // Can be found from these arrays with foliage ID.
    size_t                 foliage_max_perchunk  [MAX_FOLIAGE_TYPES];
    Model                  foliage_models [MAX_FOLIAGE_TYPES];
    struct foliage_rdata_t foliage_rdata  [MAX_FOLIAGE_TYPES]; // Used for rendering.

    float highest_point;
    float lowest_point;
    float scaling;

    float water_ylevel; // (CURRENTLY NOT USED)
    Model waterplane;   // (CURRENTLY NOT USED)

    // Triangles saved but in order to get triangle at xz location efficiently.
    struct triangle2x_t* triangle_lookup;
};

// NOTE: Coordinates are not world coordinates for this function.
// Use 'raycast_terrain' for that.
float get_heightmap_value(struct terrain_t* terrain, float x, float z);

// More optimized way to raycast the terrain instead of raycasting on the whole terrain mesh.
// it uses triangle lookup table.
// The name might be bit confusing but it actually raycasts the terrain from above.
RayCollision raycast_terrain(struct terrain_t* terrain, float x, float z);
Matrix get_rotation_to_surface(struct terrain_t* terrain, float x, float z, RayCollision* ray_out);

void delete_terrain(struct terrain_t* terrain);
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

void write_terrain_grass_positions(struct state_t* gst, struct terrain_t* terrain);
void render_terrain(struct state_t* gst, struct terrain_t* terrain, int render_pass);



#endif
