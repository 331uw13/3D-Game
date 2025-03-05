
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
#include "res/shaders/light.glsl"

void main()
{
    vec3 col;
    vec3 g_lightdot = vec3(0.0);

    vec3 texelcolor = texture(texture0, fragTexCoord).rgb * 0.5;
    texelcolor *= vec3(0.7, 0.5, 0.2);

    vec3 view_dir = normalize(fragViewPos - fragPosition);
    compute_lights(view_dir);
   

    col = texelcolor * g_lightcolor;
    col += texelcolor * AMBIENT;

    float dist = length(fragViewPos - fragPosition);
    col = get_fog(col, dist);

    finalColor = vec4(col, 1.0);
}
