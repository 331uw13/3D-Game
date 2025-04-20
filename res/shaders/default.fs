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
uniform vec3  u_campos;
uniform int   u_ground_pass; // Set to positive number if ground is being rendered.
uniform float u_terrain_size;

#define BIOMEID_COMFY 0
#define BIOMEID_HAZY 1
#define BIOMEID_EVIL 2
#define MAX_BIOME_TYPES 3

uniform sampler2D biome_groundtex[MAX_BIOME_TYPES];
uniform vec2  biome_ylevels[MAX_BIOME_TYPES];
uniform float u_terrain_lowest;
uniform float u_terrain_highest;

// Output fragment color
out vec4 finalColor;
#include "res/shaders/light.glsl"
#include "res/shaders/voronoi.glsl"
#include "res/shaders/shadow.glsl"
#include "res/shaders/fog.glsl"


#define S smoothstep


void main()
{
    finalColor = vec4(0.0, 0.0, 0.0, 1.0);
   
    vec4 texel_color = texture(texture0, fragTexCoord);


    // Biome blending.
    if(u_ground_pass == 1) {
        vec2 comfy_level = biome_ylevels[BIOMEID_COMFY];
        vec2 hazy_level = biome_ylevels[BIOMEID_HAZY];
        vec2 evil_level = biome_ylevels[BIOMEID_EVIL];

        const float Y = fragPosition.y;

        // Biome weights by Y position.
        float comfy_w = S(comfy_level.y, comfy_level.x, Y);
        float hazy_w  = S(hazy_level.y, hazy_level.x, Y);
        // Move lowest point very far away so the texture wont disappear at the bottom.
        // But has a chance to get a bit darker.
        float evil_w  = S(evil_level.y-5000, evil_level.x, Y);

        comfy_w = clamp(comfy_w, 0.0, 1.0);
        hazy_w = clamp(hazy_w, 0.0, 1.0);
        evil_w = clamp(evil_w, 0.0, 1.0);

        hazy_w *= (1.0-comfy_w);
        evil_w *= (1.0-(hazy_w+comfy_w));

        vec4 comfy_tex = texture(biome_groundtex[BIOMEID_COMFY], fragTexCoord);
        vec4 hazy_tex = texture(biome_groundtex[BIOMEID_HAZY], fragTexCoord);
        vec4 evil_tex = texture(biome_groundtex[BIOMEID_EVIL], fragTexCoord);

        texel_color = (comfy_tex * comfy_w) + (hazy_tex * hazy_w) + (evil_tex * evil_w);
    }

    vec3 normal = normalize(fragNormal);
    vec3 view_dir = normalize(u_campos - fragPosition);
    compute_lights(view_dir);

    finalColor = (texel_color * ((colDiffuse + vec4(g_lightspecular, 1.0)) * vec4(g_lightcolor,1.0)));
    finalColor.xyz += texel_color.xyz * AMBIENT;
    finalColor.xyz *= get_shadows();


    // Create effect around water.
   
    /*
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
    */

    vec3 mapped = finalColor.xyz / (finalColor.xyz + vec3(1.6));
    finalColor.xyz = pow(mapped, vec3(1.0 / 0.6));


    float dist = length(u_campos - fragPosition);
    finalColor.xyz = get_fog(finalColor.rgb, dist, _YLEVEL);
    
}




