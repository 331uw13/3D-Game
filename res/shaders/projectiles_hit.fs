#version 330

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

uniform sampler2D texture0;

out vec4 finalColor;
uniform vec3 viewPos;
uniform vec3 psystem_color;
uniform float time;


#include "res/shaders/voronoi.glsl"

void main()
{
    vec3 col = psystem_color;
    float v = voronoi3d(time*5.0 + fragPosition*0.5).x;
  
    v *= v * v;

    finalColor = vec4(col*1.2, v * 1.5);
}
