
#version 430

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;
flat in int instance_id;

uniform sampler2D texture0;

out vec4 finalColor;
uniform vec3 viewPos;


uniform vec4 psystem_color;
uniform float time;



#include "res/shaders/voronoi.glsl"



void main()
{
    float v = voronoi3d(fragPosition*0.65).x;
    vec3 col = psystem_color.rgb;

    v *= v;

    finalColor = vec4(col*1.25, v);
}
