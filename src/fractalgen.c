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
    
    fmodel->material = LoadMaterialDefault();
    fmodel->material.shader = gst->shaders[FRACTAL_MODEL_SHADER];

    fmodel->transform = MatrixTranslate(0, 0, 0);
}

void render_fractal_model(struct fractal_t* fmodel) {
    DrawMesh(
            fmodel->mesh,
            fmodel->material,
            fmodel->transform
            );
}

void delete_fractal_model(struct fractal_t* fmodel) {
    
    rlUnloadVertexArray(fmodel->mesh.vaoId);
    fmodel->mesh.vaoId = 0;

    for(int i = 0; i < /*MAX_MESH_VERTEX_BUFFERS*/9; i++) {
        rlUnloadVertexBuffer(fmodel->mesh.vboId[i]);
        fmodel->mesh.vboId[i] = 0;
    }

    free(fmodel->mesh.colors);
    free(fmodel->mesh.vertices);

    printf("%s\n", __func__);
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

    Vector3 p0 = (Vector3) {
        mtx.m12, mtx.m13, mtx.m14
    };


    Matrix offset = MatrixTranslate(0, height, 0);
    Matrix rotm = MatrixRotateXYZ(rotation);
    mtx = MatrixMultiply(offset, mtx);
    mtx = MatrixMultiply(rotm, mtx);

    Vector3 p1 = (Vector3){ 0, 0, 0 };
    p1 = Vector3Transform(p1, mtx);

    height *= dampen_height;
    add_cube_data(mtx, data, num_elems, depth, height, cube_scale);
    cube_scale *= dampen_cube_scale;

    if(depth > 0) {
        
        Vector3 rotv;

        rotv = (Vector3) {
            rotation_add.x,
            rotation_add.y,
            rotation_add.z
        };


        rotation_add.y += (M_PI/2.0) + cos(rotation.y * rotation.x);

        fractalgen_tree_branch(gst, fmodel, mtx,
                height, dampen_height, depth-1,
                rotv,
                rotation_add,
                cube_scale,
                dampen_cube_scale,
                data,
                max_elems,
                num_elems
                );

        fractalgen_tree_branch(gst, fmodel, mtx,
                height, dampen_height, depth-1,
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

    return r;
}

void fractalgen_tree(
        struct state_t* gst,
        struct fractal_t* fmodel,
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

    int depth = 12;

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
            start_rotation,
            rotation_add,
            start_cube_scale,
            dampen_cube_scale,
            data,
            max_elems,
            &num_elems
            );

    printf("\033[32m< %s >\033[0m\n", __func__);
    printf("  -> Triangles: %li\n", num_elems);


    // Fill mesh data.
    
    const int tr_count_inone = 8; // Triangle count in one shape.


    Mesh* mesh = &fmodel->mesh;

    mesh->triangleCount = num_elems * tr_count_inone;
    mesh->vertexCount = mesh->triangleCount * 3;
    mesh->vertices = malloc(mesh->vertexCount * (sizeof(float) * 3));
    mesh->texcoords = NULL;

    mesh->colors = malloc(mesh->vertexCount * (sizeof(unsigned char) * 4));

    // Used for calculating normals.
    Vector3 vA = { 0 };
    Vector3 vB = { 0 };
    Vector3 vC = { 0 };

    size_t vc = 0; // Used to count vertices.

    for(size_t i = 0; i < num_elems; i++) {
        int tr_i = 0; // Offset.
        int color_i = 0;


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

    for(int i = 0; i < mesh->vertexCount; i++) {
        int color_i = i * 4;

        float v_depth = (float)data[i / (tr_count_inone * 3)].depth;

        float t = normalize(v_depth, 0.0, (float)depth);
        Color color = ColorLerp(start_color, end_color, t);
        mesh->colors[color_i+0] = color.r;
        mesh->colors[color_i+1] = color.g;  
        mesh->colors[color_i+2] =color.b;  
        mesh->colors[color_i+3] = 255;  

    }

   
    SetTraceLogLevel(LOG_ALL);
    UploadMesh(mesh, 0);
    SetTraceLogLevel(LOG_NONE);

    printf("Fractal generation finished in %0.4f seconds\033[0m\n", GetTime() - start_time);

    free(data);


}





