#ifndef GAME_BIOME_H
#define GAME_BIOME_H

#include <raylib.h>

struct state_t;
struct terrain_t;

#include "fog.h"
#include "util.h"
#include "light.h"

#define BIOMEID_COMFY  0  // Comfortable biome to be in no much threats other than few easy robots.
#define BIOMEID_HAZY   1  // Player cant see very far and dangers may lurk outside of view.
#define BIOMEID_EVIL   2  // Full of pure evil and death.
#define MAX_BIOME_TYPES 3

#define GLSL_BIOME_STRUCT_SIZE (4*4)

struct biome_t {
    int id;
    struct fog_t fog;
    Color sun;
};


void setup_biomes(struct state_t* gst);
void setup_biome_areas(struct state_t* gst);
void update_biome_data(struct state_t* gst, struct biome_t* biome);
void change_to_biome(struct state_t* gst, int biomeid);
int  get_biomeid_by_ylevel(struct state_t* gst, float y);
const char* get_biome_name_by_id(size_t biomeid);

// Environment variable blending. fog, sun...
void update_biome_envblend(struct state_t* gst);


#endif
