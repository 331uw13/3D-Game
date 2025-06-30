#ifndef TERRAIN_CHUNK_H
#define TERRAIN_CHUNK_H

#include <raylib.h>

#include "biome.h"
#include "items/item.h"
#include "fractalgen.h"
#include "enemy.h"

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

#define MAX_ITEMS_PERCHUNK 64
#define MAX_ENEMIES_PERCHUNK 40      // Completely max number of enemies in the chunk.
#define MAX_ENEMIES_PERCHUNK_SAFE 32 // The array must have some free room left for enemies who can move around.
#define MAX_FRACTALS_PERCHUNK 32
#define FRACTAL_SPAWN_CHANCE 50  // 0 - 100

#define MAX_LIGHTS_PERCHUNK 64

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

    struct chunk_area_t area;

    struct item_t items[MAX_ITEMS_PERCHUNK];
    int num_items;

    struct enemy_t enemies[MAX_ENEMIES_PERCHUNK];
    uint16_t num_enemies;

    // The fractal will have special function when player interacts with it.
    struct fractal_t fractals[MAX_FRACTALS_PERCHUNK];
    int num_fractals;

};

void load_chunk_foliage_models(struct state_t* gst, struct terrain_t* terrain);
void setup_chunk_foliage(struct state_t* gst, struct terrain_t* terrain, struct chunk_t* chunk);
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
void position_to_chunk_index(Vector3 positon, size_t* out_index);

// --- Items ---
void chunk_add_item(struct chunk_t* chunk, struct item_t* item);
void chunk_update_items(struct state_t* gst, struct chunk_t* chunk);
void chunk_render_items(struct state_t* gst, struct chunk_t* chunk);


// --- Enemies ---

// 'adder_type' for 'chunk_add_enemy' (TODO: add overwrite option for important enemies)
#define ENEMY_ADDED_BY_SYSTEM 0 // (Enemy is spawned) Uses MAX_ENEMIES_PERCHUNK_SAFE.
#define ENEMY_ADDED_BY_CHUNK 1  // (Enemy moved from another chunk) Uses MAX_ENEMIES_PERCHUNK.
#define FIND_ENEMY_CHUNK NULL

struct enemy_t* chunk_add_enemy(
        struct state_t* gst,
        struct chunk_t* chunk,
        Vector3 position,
        int enemy_type,
        int enemy_mood,
        int adder_type
        );

// Moves enemy from enemy->chunk to new_chunk.
// Nothing is done if the chunk indices match.
void chunk_relocate_enemy(struct enemy_t* enemy, struct chunk_t* new_chunk);

void chunk_remove_enemy(struct enemy_t* enemy);
void chunk_update_enemies(struct state_t* gst, struct chunk_t* chunk);
void chunk_render_enemies(struct state_t* gst, struct chunk_t* chunk);


// --- Fractals ---
void chunk_update_fractals(struct state_t* gst, struct chunk_t* chunk);
void chunk_render_fractals(struct state_t* gst, struct chunk_t* chunk, int render_pass);


// --- Lights ---
void chunk_update_lights(struct state_t* gst, struct chunk_t* chunk);
void chunk_prepare_lights(struct state_t* gst, struct chunk_t* chunk); // Prepare lights for rendering.



// These can be used for debug if needed.
void render_chunk_borders(struct state_t* gst, struct chunk_t* chunk, Color color);


#endif
