#include <stdlib.h>
#include <stdio.h>

#include "fractalgen.h"
#include "state/state.h"



struct triangle_t {

    Vector3 a;
    Vector3 b;
    Vector3 c;

};

struct vertexdata_t {

    struct triangle_t tr[3];
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
    free(fmodel->mesh.vertices);
    free(fmodel->mesh.colors);
}


static Vector3 getvertex(Matrix mtx, float offx, float offy, float offz) {
    Matrix offset = MatrixTranslate(offx, offy, offz);
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
    tr->a = getvertex(mtx, 0, 0, 0);
    tr->b = getvertex(mtx, 0, height, 0);
    tr->c = getvertex(mtx, 0, 0, scale);
    
    tr = &data[*num_elems].tr[1];
    tr->a = getvertex(mtx, 0, 0, scale);
    tr->b = getvertex(mtx, 0, height, scale);
    tr->c = getvertex(mtx, 0, height, 0);
  



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
        float yrot_off = 0.0;

        for(int i = 0; i < 2; i++) {
            rotv = (Vector3) {
                rotation_add.x,
                rotation_add.y + yrot_off,
                rotation_add.z
            };

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

            yrot_off = M_PI/2.0;
        }

    }
}

void fractalgen_tree(struct state_t* gst, struct fractal_t* fmodel) {
    double start_time = GetTime();

    init_fractal_struct(gst, fmodel);

    int depth = 9;
    float start_height  = 10.0;
    float dampen_height = 0.75;
    float start_cube_scale = 1.5;
    float dampen_cube_scale = 0.6;

    Vector3 start_rotation = (Vector3){ 0, 0, 0 };
    Vector3 rotation_add = (Vector3){ 0.67, 0.0, 0.0 };

    size_t max_elems = 1600000;
    struct vertexdata_t* data = malloc(max_elems * sizeof *data);
    size_t num_elems = 0;

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

    printf("\033[32m --- FractalGen --- \033[0m\n");
    printf("  -> Triangles: %li\n", num_elems);


    // Fill mesh vertices (TODO: Calculate normals.)

    Mesh* mesh = &fmodel->mesh;

    size_t vc = 0; // Used to count vertices.
    const int tr_count_inone = 2; // Triangle count in one shape.


    mesh->triangleCount = num_elems;
    mesh->vertexCount = mesh->triangleCount * (3 * tr_count_inone);
    mesh->vertices = malloc(mesh->vertexCount * (sizeof(float) * 3));
    mesh->normals = NULL;
    mesh->texcoords = NULL;

    mesh->colors = malloc(mesh->vertexCount * (sizeof(unsigned char) * 4));



    for(size_t i = 0; i < num_elems; i++) { // All triangles.
        int tr_i = 0; // Offset.
        int color_i = 0;


        for(int k = 0; k < tr_count_inone; k++) { // One triangle.

            // Vertices.

            struct triangle_t* tr = &data[i].tr[k];
            mesh->vertices[vc+0+tr_i] = tr->a.x;
            mesh->vertices[vc+1+tr_i] = tr->a.y;
            mesh->vertices[vc+2+tr_i] = tr->a.z;
            
            mesh->vertices[vc+3+tr_i] = tr->b.x;
            mesh->vertices[vc+4+tr_i] = tr->b.y;
            mesh->vertices[vc+5+tr_i] = tr->b.z;
            
            mesh->vertices[vc+6+tr_i] = tr->c.x;
            mesh->vertices[vc+7+tr_i] = tr->c.y;
            mesh->vertices[vc+8+tr_i] = tr->c.z;
        
            tr_i += 9;
        }

        vc += (9 * tr_count_inone);
    }


    Color start_color   = (Color){ 30, 80, 60, 255 };
    Color end_color = (Color){ 80, 40, 30, 255 };

    for(int i = 0; i < mesh->vertexCount; i++) {
        int color_i = i * 4;

        float v_depth = (float)data[i / 6].depth;

        float t = normalize(v_depth, 0.0, (float)depth);
        Color color = ColorLerp(start_color, end_color, t);
        mesh->colors[color_i+0] = color.r;
        mesh->colors[color_i+1] = color.g;  
        mesh->colors[color_i+2] =color.b;  
        mesh->colors[color_i+3] = 255;  

    }

    

    UploadMesh(mesh, 0);

    printf("Fractal generation finished in %0.4f seconds\033[0m\n", GetTime() - start_time);

    free(data);


}





