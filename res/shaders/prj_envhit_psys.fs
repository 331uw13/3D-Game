
#version 430

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

uniform sampler2D texture0;

out vec4 finalColor;
uniform vec3 viewPos;


uniform vec4 psystem_color;
uniform float time;


#include "res/shaders/voronoi.glsl"

void main()
{
    vec3 col = psystem_color.rgb;
    float v = voronoi3d(vec3(0.0, time*5, 0.0) + fragPosition*0.35).x;
  
    v *= v;

    finalColor = vec4(col, v);
}
