#include <stdlib.h>
#include <stdio.h>

#include "fractalgen.h"
#include "state/state.h"

#include <rlgl.h>


struct triangle_t {

    Vector3 a;
    Vector3 b;
    Vector3 c;

};

struct vertexdata_t {

    struct triangle_t tr[8];
    int depth;
};

static void init_fractal_struct(struct state_t* gst, struct fractal_t* fmodel) {
    fmodel->num_berries = 0; 
    fmodel->material = LoadMaterialDefault();
    fmodel->material.shader = gst->shaders[FRACTAL_MODEL_SHADER];
    fmodel->transform = MatrixTranslate(0, 0, 0);
}

void delete_fractal_model(struct fractal_t* fmodel) {
    
    rlUnloadVertexArray(fmodel->mesh.vaoId);
    fmodel->mesh.vaoId = 0;

    for(int i = 0; i < 9; i++) {
        rlUnloadVertexBuffer(fmodel->mesh.vboId[i]);
        fmodel->mesh.vboId[i] = 0;
    }

    free(fmodel->mesh.colors);
    free(fmodel->mesh.vertices);
}


static Vector3 getvertex(Matrix mtx, float scale, float offx, float offy, float offz) {
    Matrix offset = MatrixTranslate(
            offx - scale*0.5,
            offy,
            offz - scale*0.5
            );

    mtx = MatrixMultiply(offset, mtx);

    Vector3 v = (Vector3){ 0 };
    v = Vector3Transform(v, mtx);
    
    return v;
}


static void add_cube_data(
        Matrix mtx,
        struct vertexdata_t* data,
        size_t* num_elems,
        int depth,
        float height,
        float scale
){
    struct triangle_t* tr = NULL;
    data[*num_elems].depth = depth;

    tr = &data[*num_elems].tr[0];
    tr->a = getvertex(mtx,scale,   0,     0,      0);
    tr->b = getvertex(mtx,scale,   0,     height, 0);
    tr->c = getvertex(mtx,scale,   0,     0,      scale);
    
    tr = &data[*num_elems].tr[1];
    tr->a = getvertex(mtx,scale,   0,     0,      scale);
    tr->b = getvertex(mtx,scale,   0,     height, scale);
    tr->c = getvertex(mtx,scale,   0,     height, 0); 

    tr = &data[*num_elems].tr[2];
    tr->a = getvertex(mtx,scale,   0,     0,      scale);
    tr->b = getvertex(mtx,scale,   0,     height, scale);
    tr->c = getvertex(mtx,scale,   scale, height, scale);

    tr = &data[*num_elems].tr[3];
    tr->a = getvertex(mtx,scale,   scale, height, scale);
    tr->b = getvertex(mtx,scale,   0,     0,      scale);
    tr->c = getvertex(mtx,scale,   scale, 0,      scale);

    tr = &data[*num_elems].tr[4];
    tr->a = getvertex(mtx,scale,   scale, 0,      scale);
    tr->b = getvertex(mtx,scale,   scale, height, scale);
    tr->c = getvertex(mtx,scale,   scale, height, 0);

    tr = &data[*num_elems].tr[5];
    tr->a = getvertex(mtx,scale,   scale, height, 0);
    tr->b = getvertex(mtx,scale,   0,     height, 0);
    tr->c = getvertex(mtx,scale,   scale, 0,      0);

    tr = &data[*num_elems].tr[6];
    tr->a = getvertex(mtx,scale,   scale, 0,      0);
    tr->b = getvertex(mtx,scale,   0,     height, 0);
    tr->c = getvertex(mtx,scale,   0,     0,      0);

    tr = &data[*num_elems].tr[7];
    tr->a = getvertex(mtx,scale,   scale, 0,      scale);
    tr->b = getvertex(mtx,scale,   scale, height, 0);
    tr->c = getvertex(mtx,scale,   scale, 0,      0);

    *num_elems += 1;
}


// Based on phytagoras tree fractal.

void fractalgen_tree_branch(
        struct state_t* gst,
        struct fractal_t* fmodel,
        Matrix mtx,
        float height,
        float dampen_height,
        int depth,
        int max_depth,
        Vector3 rotation,
        Vector3 rotation_add,
        float cube_scale,
        float dampen_cube_scale,

        struct vertexdata_t* data,
        const size_t   max_elems,
        size_t*        num_elems
){

    if(*num_elems+1 >= max_elems) {
        fprintf(stderr, "\033[31m(ERROR) '%s': Not enough memory allocated for triangles!\033[0m\n",
                __func__);
        return;
    }


    Matrix offset = MatrixTranslate(0, height, 0);
    Matrix rotm = MatrixRotateXYZ(rotation);
    mtx = MatrixMultiply(offset, mtx);
    mtx = MatrixMultiply(rotm, mtx);

    add_cube_data(mtx, data, num_elems, depth, height, cube_scale);
    height *= dampen_height;
    cube_scale *= dampen_cube_scale;


    // Branch has a chance to generate one berry
    if(depth > 1 && depth < (max_depth-2)
    && *num_elems > 0 /* depth checks should avoid this but just to be safe. */
    ){
        if((fmodel->num_berries+1 < MAX_BERRIES_PER_FRACTAL)
        && (RSEEDRANDOMF(0, 100) < 10)) {
           
            fmodel->berries[fmodel->num_berries] = (struct berry_t) {
                .position = data[*num_elems-1].tr[0].a,
                .level = RSEEDRANDOMF(-MIN_BERRY_LEVEL, MAX_BERRY_LEVEL)
            };
            fmodel->num_berries++;
        }       
    }

    if(depth > 0) {
 


        Vector3 rotv;

        rotv = (Vector3) {
            rotation_add.x,
            rotation_add.y,
            rotation_add.z
        };


        rotation_add.y += (M_PI/2.0) + cos(rotation.y * rotation.x);

        fractalgen_tree_branch(gst, fmodel, mtx,
                height, dampen_height, depth-1, max_depth,
                rotv,
                rotation_add,
                cube_scale,
                dampen_cube_scale,
                data,
                max_elems,
                num_elems
                );

        fractalgen_tree_branch(gst, fmodel, mtx,
                height, dampen_height, depth-1, max_depth,
                Vector3Negate(rotv),
                rotation_add,
                cube_scale,
                dampen_cube_scale,
                data,
                max_elems,
                num_elems
                );

    }
}

static Vector3 get_random_rotation_adder(struct state_t* gst, Vector3 weights) {

    float rX = M_PI * weights.x;
    float rY = M_PI * weights.y;
    float rZ = M_PI * weights.z;

    Vector3 r = (Vector3){
        RSEEDRANDOMF(-rX, rX),
        RSEEDRANDOMF(-rY, rY),
        RSEEDRANDOMF(-rZ, rZ)
    };

    if(r.x <= 0.01) {
        r.x = 0.1;
    }

    return r;
}

void fractalgen_tree(
        struct state_t* gst,
        struct fractal_t* fmodel,
        int depth,
        Vector3 rotation_weights,
        float start_height,
        float dampen_height,
        float start_cube_scale,
        float dampen_cube_scale,
        Color start_color,
        Color end_color
){
    double start_time = GetTime();

    init_fractal_struct(gst, fmodel);

    Vector3 start_rotation = (Vector3){ 0, 0, 0 };
    Vector3 rotation_add = get_random_rotation_adder(gst, rotation_weights);

    size_t max_elems = 1600000;
    struct vertexdata_t* data = malloc(max_elems * sizeof *data);
    size_t num_elems = 0; // "How many cubes" (TODO: Rename.)

    fractalgen_tree_branch(gst,
            fmodel,
            MatrixTranslate(0, 0, 0),
            start_height,
            dampen_height,
            depth,
            depth,
            start_rotation,
            rotation_add,
            start_cube_scale,
            dampen_cube_scale,
            data,
            max_elems,
            &num_elems
            );

    // Fill mesh->data.
    
    const int tr_count_inone = 8; // Triangle count in one shape.


    Mesh* mesh = &fmodel->mesh;

    mesh->triangleCount = num_elems * tr_count_inone;
    mesh->vertexCount = mesh->triangleCount * 3;
    mesh->vertices = malloc(mesh->vertexCount * (sizeof(float) * 3));
    mesh->texcoords = NULL;
    mesh->normals = NULL;
    mesh->tangents = NULL;
    mesh->colors = NULL;
    mesh->indices = NULL;
    mesh->animVertices = NULL;
    mesh->animNormals = NULL;
    mesh->boneIds = NULL;
    mesh->boneWeights = NULL;
    mesh->boneMatrices = NULL;
    mesh->boneCount = 0;
    mesh->colors = malloc(mesh->vertexCount * (sizeof(unsigned char) * 4));

    size_t vc = 0; // Used to count vertices.

    for(size_t i = 0; i < num_elems; i++) {
        int tr_i = 0; // Offset.


        for(int k = 0; k < tr_count_inone; k++) { 

            // Vertices.

            struct triangle_t* tr = &data[i].tr[k];
            mesh->vertices[vc+0+tr_i] = tr->a.x;
            mesh->vertices[vc+1+tr_i] = tr->a.y - start_height;
            mesh->vertices[vc+2+tr_i] = tr->a.z;
            
            mesh->vertices[vc+3+tr_i] = tr->b.x;
            mesh->vertices[vc+4+tr_i] = tr->b.y - start_height;
            mesh->vertices[vc+5+tr_i] = tr->b.z;
            
            mesh->vertices[vc+6+tr_i] = tr->c.x;
            mesh->vertices[vc+7+tr_i] = tr->c.y - start_height;
            mesh->vertices[vc+8+tr_i] = tr->c.z;


            tr_i += 9;
        }

        vc += (9 * tr_count_inone);
    }


    // Colors.
    for(int i = 0; i < mesh->vertexCount; i++) {
        int color_i = i * 4;

        float v_depth = (float)data[i / (tr_count_inone * 3)].depth;

        float t = normalize(v_depth, 0.0, (float)depth);
        Color color = ColorLerp(end_color, start_color, t);
        mesh->colors[color_i+0] = color.r;
        mesh->colors[color_i+1] = color.g;  
        mesh->colors[color_i+2] = color.b;  
        mesh->colors[color_i+3] = 255;  
    }

   
    //SetTraceLogLevel(LOG_ALL);
    UploadMesh(mesh, 0);
    //SetTraceLogLevel(LOG_NONE);


    //printf("Fractal generation finished in %0.4f seconds\033[0m\n", GetTime() - start_time);
    free(data);
}





