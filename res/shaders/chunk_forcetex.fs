#version 430

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;

out vec4 finalColor;


// NOTE: This must be same as in 'src/state/state.h'
#define MAX_GRASS_FORCEVECTORS 16 


layout (std140, binding = 6) uniform force_vec_ub {
    // XYZ = Position
    // W = Strength
    vec4 force_vectors[MAX_GRASS_FORCEVECTORS];
};

uniform float u_chunk_size;
uniform vec2 u_terrain_origin;
uniform vec2 u_chunk_coords;
uniform float u_grass_spacing;
uniform float u_terrain_size;

float map(float t, float src_min, float src_max, float dst_min, float dst_max) {
    return (t - src_min) * (dst_max - dst_min) / (src_max - src_min) + dst_min;
}



void main()
{
    finalColor = vec4(0.0, 0.0, 0.0, 1.0);
    
    float chunk_size = float(u_chunk_size);
    //vec2 chunk_coords = u_chunk_coords - u_terrain_origin;


    for(int i = 0; i < MAX_GRASS_FORCEVECTORS; i++) {
        if(force_vectors[i].w <= 0.0) {
            continue;
        }

        vec2 fd = (gl_FragCoord.xy) / 128.0;
        vec2 d = (force_vectors[i].xz - u_chunk_coords) / chunk_size;

        finalColor.r = length(fd - d);
        break;
    }

}



