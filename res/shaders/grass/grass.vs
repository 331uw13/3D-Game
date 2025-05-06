#version 430 core


in vec3 vertexPosition;

out vec3 fragNormal_f;
out vec3 fragPosition;
out vec3 force_vector;
out float test_shit;

out float grassblade_base_y; // Terrain ylevel at grass blade position.

uniform mat4 u_viewproj;
uniform int u_chunk_grass_baseindex;
uniform float u_time;


#include "res/shaders/grass/grassdata.glsl"



mat3 rotate_m3(vec2 ang) {
    vec2 c = cos(ang);
    vec2 s = sin(ang);
    return mat3(
            c.y,      0.0, -s.y,
            s.y*s.x,  c.x,  c.y*s.x,
            s.y*c.x, -s.x,  c.y*c.x);
}

float ease_outcubic(float x) {
    return 1.0 - pow(1.0 - x, 3);
}

#define PI 3.14159


void main()
{
    uint id = gl_InstanceID + u_chunk_grass_baseindex;
    fragPosition = grassdata[id].position.xyz;
    grassblade_base_y = fragPosition.y;

    
    float bend = ((fragPosition.y+vertexPosition.y) - grassblade_base_y) / 10.0;
    bend *= GRASSDATA_BEND_VALUE(id);


    mat3 rotation = mat3(grassdata[id].rotation);
    mat3 bend_rotation = rotate_m3(vec2(-bend, -1.5));

    // Random rotation so the blades are not facing exactly the same direction.
    float rnd = sin((fract(grassdata[id].position.x)*fract(grassdata[id].position.z))*100);
    mat3 rnd_rotation = rotate_m3(vec2(0.0, rnd));
    

    /*
    // Calculate rotation away from force vector..
    {

        vec3 diff = grassdata[id].ext_force.xyz - grassdata[id].position.xyz;
        float ang = -(atan(diff.x, diff.z));

        float dist = length(grassdata[id].position.xyz - grassdata[id].ext_force.xyz);
        dist = clamp(dist, 0.0, 80.0);
        dist = 1.0 - (dist / 80.0);

        mat3 rot 
            = rotate_m3(vec2(dist * (bend * 3.5), 0.0))
            * rotate_m3(vec2(bend,        ang * (dist)));
        rotation *= rot;
    }
    */

    // TODO: Rename.
    vec3 test = vec3(
            sin(u_time*5.0 + vertexPosition.y*10.0)*0.25,
            0.0,
            0.0
            );

    fragPosition += (test + vertexPosition) * (rnd_rotation *  ((bend_rotation) * (rotation)));
    gl_Position = u_viewproj * vec4(fragPosition, 1.0);

}






