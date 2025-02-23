#ifndef TERRAIN_H
#define TERRAIN_H

#include <raylib.h>

#include "perlin_noise.h"
#include "typedefs.h"
#include <stddef.h>

struct state_t;


#define HEIGHTMAP_SIZE_X 300
#define HEIGHTMAP_SIZE_Z 300
#define HEIGHTMAP_SIZE_XZ (HEIGHTMAP_SIZE_X * HEIGHTMAP_SIZE_Z)


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

struct terrain_t {
    Mesh      mesh;
    Material  material;
    Matrix    transform;
    int       mesh_generated;
    struct heightmap_t heightmap;
 
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
        int    octaves
        );

void delete_terrain    (struct terrain_t* terrain);

//void generate_heightmap(struct terrain_t* terrain);
//void generate_terrain_mesh(struct state_t* gst, struct terrain_t* terrain);

void render_terrain(struct state_t* gst, struct terrain_t* terrain);




#endif
