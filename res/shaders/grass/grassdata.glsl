

// Remember to adjust 
// 'GRASSDATA_STRUCT_SIZE' and 'GRASSDATA_NUM_FLOATS_RESERVED'
// If adding more elements to grassdata_t structure.
// They are defined in 'state/state.h'

struct grassdata_t {
    vec4 position;
    
    // X = How much the grass should bend?
    // Y = ..
    // Z = ..
    // W = ..
    vec4 settings;

    mat3x4 rotation;

};

layout(std430, binding = 5) buffer grassdata_ssbo {
    grassdata_t grassdata[];
};


#define GRASSDATA_BEND_VALUE(id) grassdata[id].settings.x



