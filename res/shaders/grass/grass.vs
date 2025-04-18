#version 430 core


in vec3 vertexPosition;

out vec3 fragNormal_f;
out vec3 fragPosition;
out vec3 campos;

out float grassblade_base_y; // Terrain ylevel at grass blade position.

uniform mat4 u_viewproj;
uniform int u_chunk_grass_baseindex;
uniform vec3 u_campos;

#include "res/shaders/grass/grassdata.glsl"


mat3 rotate_m3(vec2 ang) {
    vec2 c = cos(ang);
    vec2 s = sin(ang);
    return mat3(
            c.y,      0.0, -s.y,
            s.y*s.x,  c.x,  c.y*s.x,
            s.y*c.x, -s.x,  c.y*c.x);
}


void main()
{
    uint id = gl_InstanceID + u_chunk_grass_baseindex;
    campos = u_campos;
    fragPosition = grassdata[id].position.xyz;
    grassblade_base_y = fragPosition.y;

    
    float bending = ((vertexPosition.y + fragPosition.y) - grassblade_base_y) / 7.0;
    bending *= GRASSDATA_BEND_VALUE(id);

    // Random rotation so they are not facing the same direction.
    float random_rotation = sin(32*fract(fragPosition.x)*12*fract(fragPosition.y));
    fragPosition += vertexPosition * rotate_m3(vec2(-bending, random_rotation));
    
    gl_Position = u_viewproj * vec4(fragPosition, 1.0);
}

