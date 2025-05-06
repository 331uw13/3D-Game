#version 430

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;

out highp vec4 finalColor;


// NOTE: This must be same as in 'src/state/state.h'
#define MAX_GRASS_FORCEVECTORS 64 


layout (std140, binding = 6) uniform force_vec_ub {
    // XYZ = Position
    // W = Strength
    vec4 force_vectors[MAX_GRASS_FORCEVECTORS];
};

uniform int u_chunk_size;
uniform vec2 u_terrain_origin;
uniform vec2 u_chunk_coords;
uniform float u_grass_spacing;
uniform float u_terrain_scaling;


float map(float t, float src_min, float src_max, float dst_min, float dst_max) {
    return (t - src_min) * (dst_max - dst_min) / (src_max - src_min) + dst_min;
}

#define PI 3.14159



void main()
{
    finalColor = vec4(0.0, 0.0, 0.0, 1.0);
    
    float chunk_size_scaled = float(u_chunk_size) * u_terrain_scaling;

    for(int i = 0; i < MAX_GRASS_FORCEVECTORS; i++) {
        if(force_vectors[i].w <= 0.0) {
            continue;
        }

        // Normalize coordinates.
        vec2 p = (gl_FragCoord.xy) / float(u_chunk_size);
        vec2 fv = (force_vectors[i].xz - u_chunk_coords) / chunk_size_scaled;


        float dist = length(p - fv) * 20.0;
        dist = 1.0 - clamp(dist, 0.0, 1.0);

        finalColor.r += dist;

        /*
        // Radius.
        float strength = 45.0 - force_vectors[i].w;
        float dist = length(p - fv) * max(strength, 1.0);
        dist = clamp(dist, 0.0, 1.0);

        float sr = step(dist, 0.5);
        finalColor.xyz += force_vectors[i].xyz * sr;
        */
    }
}



