#version 430

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;


// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float u_time;
uniform vec3 u_campos;

// Output fragment color
out vec4 finalColor;

#include "res/shaders/light.glsl"
#include "res/shaders/voronoi.glsl"
#include "res/shaders/fog.glsl"



float lerp(float t, float min, float max) {
    return min + t * (max - min);
}



void main()
{
    vec4 texelColor = vec4(0.0, 0.8, 1.0, 1.0);

    float v = voronoi3d(fragPosition.xyz*0.005 + vec3(sin(u_time)*0.1, u_time*0.35, cos(u_time)*0.1)).x;
    float v2 = voronoi3d(fragPosition.xyz*0.0065 + vec3(sin(u_time)*0.5, u_time*0.25, cos(u_time)*0.183)).y;
    texelColor.rgb += v + v2*0.8;


    vec3 normal = normalize(fragNormal);
    vec3 view_dir = normalize(u_campos - fragPosition);

  
    compute_lights(view_dir);

    finalColor = (texelColor * ((colDiffuse + vec4(g_lightspecular, 1.0)) * vec4(g_lightcolor,1.0)));
    finalColor.xyz += texelColor.xyz * AMBIENT;


    vec3 mapped = finalColor.xyz / (finalColor.xyz + vec3(1.6));
    finalColor.xyz = pow(mapped, vec3(1.0 / 0.6));


    float dist = length(u_campos - fragPosition);
    finalColor.xyz = get_fog(finalColor.rgb, dist, _YLEVEL);

    finalColor.w = 1.0;
}
