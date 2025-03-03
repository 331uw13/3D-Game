#version 330

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

uniform sampler2D texture0;

out vec4 finalColor;
uniform vec3 viewPos;
uniform vec3 psystem_color;


#include "res/shaders/voronoi.glsl"

void main()
{
    vec3 col = psystem_color;
    float v = voronoi3d(fragPosition*2.0).x;
  
    v *= v;

    finalColor = vec4(col, v);
}
