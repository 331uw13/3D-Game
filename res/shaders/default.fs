#version 430

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;


// Input uniform values
uniform sampler2D texture0;
uniform sampler2D u_defnoise_tex;

uniform vec4 colDiffuse;
uniform float u_waterlevel;
uniform float terrain_lowest_point;
uniform float u_time;
uniform float u_shadowcam_y;
uniform vec3 u_campos;


// Output fragment color
out vec4 finalColor;
#include "res/shaders/fog.glsl"
#include "res/shaders/light.glsl"
#include "res/shaders/voronoi.glsl"
#include "res/shaders/shadow.glsl"
/*
float ld(float depth) {
    float near = 0.1;
    float far = 2000.0*u_shadow_tresh;
    float ndc = depth;// * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - ndc * (far - near)) / far;
}
*/

// TODO: Fix shadowcam flip near water(???)

void main()
{
    finalColor = vec4(0.1, 0.0, 0.1, 1.0);
   
    vec4 texel_color = texture(texture0, fragTexCoord);
    vec3 normal = normalize(fragNormal);
  
    vec3 view_dir = normalize(u_campos - fragPosition);
    compute_lights(view_dir);

    finalColor = (texel_color * ((colDiffuse + vec4(g_lightspecular, 1.0)) * vec4(g_lightcolor,1.0)));
    finalColor.xyz += texel_color.xyz * AMBIENT;
    finalColor.xyz *= get_shadows();

    vec3 mapped = finalColor.xyz / (finalColor.xyz + vec3(1.6));
    finalColor.xyz = pow(mapped, vec3(1.0 / 0.6));

    // Create effect around water.
    
    float rad = 6.24;
    float level = (u_waterlevel+rad) + voronoi3d(fragPosition.xyz*0.01+vec3(0,-u_time,0)).y*5.0;

    float y = fragPosition.y;
    if(y <= level && y >= u_waterlevel) {
        // Color above water.

        float t = (y - level) / (u_waterlevel - level);

        vec3 to = vec3(0.0, 0.1, 0.3);
        vec3 from = vec3(0.0, 0.5, 0.7);

        finalColor.xyz += 0.5*vec3(
                _lerp(t, to.x, from.x),
                _lerp(t, to.y, from.y),
                _lerp(t, to.z, from.z)
                ) * t;
    }
    else
    if(y <= u_waterlevel) {
        // Color below water.
        
        float min = u_waterlevel;
        float max = terrain_lowest_point;
        float t = (y - min) / (max - min);
        t = clamp(t, 0.0, 1.0);

        vec3 from = vec3(0.2, 0.1, 0.3);
        vec3 to = vec3(0.0, 0.3, 0.4);

        finalColor.xyz += 0.5*vec3(
                _lerp(t, to.x, from.x),
                _lerp(t, to.y, from.y),
                _lerp(t, to.z, from.z)
                );
    }



    float dist = length(u_campos - fragPosition);
    finalColor.xyz = get_fog(finalColor.rgb, dist, _YLEVEL);
}




