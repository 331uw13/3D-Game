#version 430

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;

in vec3 fragViewPos;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float water_level;
uniform float time;

// Output fragment color
out vec4 finalColor;
#include "res/shaders/fog.glsl"
#include "res/shaders/light.glsl"
#include "res/shaders/voronoi.glsl"



float lerp(float t, float min, float max) {
    return min + t * (max - min);
}

float mapt(float t, float src_min, float src_max, float dst_min, float dst_max) {
    return (t - src_min) * (dst_max - dst_min) / (src_max - src_min) + dst_min;
}


void main()
{

    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);
    

    vec3 normal = normalize(fragNormal);
    vec3 view_dir = normalize(fragViewPos - fragPosition);

  
    compute_lights(view_dir);

    finalColor = (texelColor * ((colDiffuse + vec4(g_lightspecular, 1.0)) * vec4(g_lightcolor,1.0)));
    finalColor.xyz += texelColor.xyz * AMBIENT;


    vec3 mapped = finalColor.xyz / (finalColor.xyz + vec3(1.6));
    finalColor.xyz = pow(mapped, vec3(1.0 / 0.6));

    // Create effect around water.
    
    float rad = 6.24;
    float level = (water_level+rad) + voronoi3d(fragPosition.xyz*0.01+vec3(0,-time,0)).y*5.0;

    float y = fragPosition.y;
    if(y <= level && y >= water_level) {
        // Color above water.

        float t = (y - level) / (water_level - level);

        vec3 to = vec3(0.0, 0.1, 0.3);
        vec3 from = vec3(0.0, 0.3, 0.4);

        finalColor.xyz += 0.5*vec3(
                lerp(t, to.x, from.x),
                lerp(t, to.y, from.y),
                lerp(t, to.z, from.z)
                ) * t;
    }
    else
    if(y <= water_level) {
        // Color below water.
        
        float min = water_level;
        float max = -300;
        float t = (y - min) / (max - min);
        t = clamp(t, 0.0, 1.0);

        vec3 from = vec3(0.2, 0.1, 0.3);
        vec3 to = vec3(0.0, 0.3, 0.4);

        finalColor.xyz += 0.5*vec3(
                lerp(t, to.x, from.x),
                lerp(t, to.y, from.y),
                lerp(t, to.z, from.z)
                );
    }


    float dist = length(fragViewPos - fragPosition);
    finalColor.xyz = get_fog(finalColor.rgb, dist);



}




