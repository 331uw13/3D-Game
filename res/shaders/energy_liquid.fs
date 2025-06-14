#version 430

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;

out vec4 finalColor;

uniform vec4 u_energy_color;
uniform float u_energy_container_level;
uniform float u_energy_container_capacity;
uniform float u_time;


float map(float t, float src_min, float src_max, float dst_min, float dst_max) {
    return (t - src_min) * (dst_max - dst_min) / (src_max - src_min) + dst_min;
}

#include "res/shaders/voronoi.glsl"


void main()
{
    finalColor = vec4(0.0, 0.0, 0.0, 1.0);
    vec3 color = u_energy_color.rgb;


    float level = u_energy_container_level / u_energy_container_capacity;
    //color *= step(1.0-fragTexCoord.y, level);
    color *= pow(smoothstep(0.0, 1.0-fragTexCoord.y, level), 32);

    float noise = voronoi3d(fragTexCoord.xyy*4.65+vec3(0.0,u_time*0.5,0.0)).x;
    color *= noise+0.35;

    finalColor.rgb = color * 0.6;
}



