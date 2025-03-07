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

// Output fragment color
out vec4 finalColor;

#include "res/shaders/fog.glsl"
#include "res/shaders/light.glsl"





float lerp(float t, float min, float max) {
    return min + t * (max - min);
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
   

    float dist = length(fragViewPos - fragPosition);
    finalColor.xyz = get_fog(finalColor.rgb, dist);
}
