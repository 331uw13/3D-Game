
#version 430

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

uniform sampler2D texture0;
uniform float time;

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


    vec4 texelcolor = texture(texture0, fragTexCoord * 2.0);
    texelcolor.rgb *= 0.6;
    //texelcolor *= vec3(0.5, 0.7, 0.2);

    vec3 view_dir = normalize(fragViewPos - fragPosition);
    compute_lights(view_dir);


    col = texelcolor.rgb * g_lightcolor;
    col += texelcolor.rgb * AMBIENT;
    float v = voronoi3d(fragPosition*0.1 - (time*0.3)*vec3(0,1,0)).x;
    v *= v;
    col += (v * 0.25) * vec3(0.0, 0.3,0.3);

    float dist = length(fragViewPos - fragPosition);
    col = get_fog(col, dist);

    finalColor = vec4(col, texelcolor.w);
}
