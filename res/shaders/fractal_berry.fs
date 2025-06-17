
#version 430

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

uniform sampler2D texture0;
uniform vec3 u_campos;

out vec4 finalColor;
uniform vec4 u_berry_color;
//uniform vec3 viewPos;


#include "res/shaders/voronoi.glsl"
#include "res/shaders/fog.glsl"

void main()
{
    vec3 col;
   
    col = u_berry_color.rgb;
    finalColor = vec4(col, 1.0);
}
