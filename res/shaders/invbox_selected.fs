
#version 430

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

uniform sampler2D texture0;
uniform vec3 u_campos;

out vec4 finalColor;


#include "res/shaders/voronoi.glsl"

void main()
{
    vec3 col;
   
    finalColor = vec4(0.3, 1.0, 0.5, 0.6);
}
