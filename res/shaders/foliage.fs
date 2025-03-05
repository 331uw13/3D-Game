
#version 430

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

uniform sampler2D texture0;

out vec4 finalColor;
//uniform vec3 viewPos;

in vec3 fragViewPos;


#include "res/shaders/fog.glsl"
#include "res/shaders/voronoi.glsl"



void main()
{
    vec3 col;

    col = texture(texture0, fragTexCoord).rgb * 0.5;
    col *= vec3(0.7, 0.5, 0.2);


    /*
    vec3 lightdir = normalize(vec3(1.0, 1.0, 1.0) - fragPosition);
    float NdotL = max(dot(fragNormal, lightdir), 0.1);
    vec3 g_lightdot = (vec3(1.0, 0.3, 0.3) * NdotL);
    col *= g_lightdot;
    */




    float dist = length(fragViewPos - fragPosition);
    col = get_fog(col, dist);

    finalColor = vec4(col, 1.0);
}
