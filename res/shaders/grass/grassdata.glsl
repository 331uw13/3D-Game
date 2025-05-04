

struct grassdata_t {
    vec4 position;
    
    // X = How much the grass should bend?
    // Y = ..
    // Z = ..
    // W = Force vector radius.
    vec4 settings;


    mat3x4 rotation;

};

layout(std430, binding = 5) buffer grassdata_ssbo {
    grassdata_t grassdata[];
};


#define GRASSDATA_BEND_VALUE(id) grassdata[id].settings.x
#define GRASSDATA_FVEC_RADIUS(id) grassdata[id].settings.w



