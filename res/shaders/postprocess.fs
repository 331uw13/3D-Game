
#version 430


// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform sampler2D bloomtresh_texture;
uniform sampler2D ssao_texture;
uniform sampler2D depth_texture;

uniform vec4 colDiffuse;
uniform float u_time;
uniform float health; // normalized
uniform vec2 u_screen_size;
uniform int u_ssao_enabled;
uniform int u_anygui_open;

// Output fragment color
out vec4 finalColor;

#define Pi 3.14159
#define Pi2 (Pi*2)
#define PiR (Pi/180.0)

float lerp(float t, float min, float max) {
    return (max - min) * t + min;
}


#include "res/shaders/voronoi.glsl"

#define BLOOM_SAMPLES 30.0
#define BLOOM_POS_M 0.5
#define BLOOM_ADD_M 0.6

//vec3 get_bloom() {
   
    /*
    vec2 texelsize = 1.0/(u_screen_size*0.35);
    vec2 offset;

    
    offset = vec2(0, 0) * texelsize;
    vec3 c0 = texture(bloomtresh_texture, fragTexCoord + offset).rgb;

    offset = vec2(1, 0) * texelsize;
    vec3 c1 = texture(bloomtresh_texture, fragTexCoord + offset).rgb;
    
    offset = vec2(-1, 0) * texelsize;
    vec3 c2 = texture(bloomtresh_texture, fragTexCoord + offset).rgb;
    
    offset = vec2(0, 1) * texelsize;
    vec3 c3 = texture(bloomtresh_texture, fragTexCoord + offset).rgb;

    offset = vec2(0, -1) * texelsize;
    vec3 c4 = texture(bloomtresh_texture, fragTexCoord + offset).rgb;
    
    offset = vec2(1, 1) * texelsize;
    vec3 c5 = texture(bloomtresh_texture, fragTexCoord + offset).rgb;
    
    offset = vec2(1, -1) * texelsize;
    vec3 c6 = texture(bloomtresh_texture, fragTexCoord + offset).rgb;
    
    offset = vec2(-1, 1) * texelsize;
    vec3 c7 = texture(bloomtresh_texture, fragTexCoord + offset).rgb;
    

    vec3 res = (c0+c1+c2+c3+c4+c5+c6+c7) / 9.0;

    return res;
    */
    /*
    vec3 result = vec3(0);
    vec2 size = u_screen_size * 0.365;
    vec2 sf = 1.0/(size * 2.0);
    const int r = 4;


    // TODO: Apply bloom based on depth.
    // Issue: stuff blurred further away looks kind of bad

    for(int x = -r; x <= r; x++) {
        for(int y = -r; y <= r; y++) {
            vec2 p = vec2(x, y) * BLOOM_POS_M;
            result += BLOOM_ADD_M * texture(bloomtresh_texture, fragTexCoord + p * sf).rgb;
        }     
    }


    for(int y = -r; y <= r; y++) {
        for(int x = -r; x <= r; x++) {
            vec2 p = vec2(x, y) * BLOOM_POS_M;
            result += BLOOM_ADD_M * texture(bloomtresh_texture, fragTexCoord + p * sf).rgb;
        }     
    }


    return (result / BLOOM_SAMPLES) * colDiffuse.rgb;
    */
//}



void main()
{
    //finalColor = texture(ssao_texture, fragTexCoord); return;

    //finalColor = texture(bloomtresh_texture, fragTexCoord); return;


    vec2 texcoords = fragTexCoord;
    vec3 color = texture(texture0, texcoords).rgb;

    vec3 bloom = texture(bloomtresh_texture, fragTexCoord).rgb;
    color += bloom;
    

    if(u_anygui_open == 1) {
        vec2 texelsize = 1.0/(u_screen_size*0.5);
        const int r = 3;
        for(int x = -r; x <= r; x++) {
            for(int y = -r; y <= r; y++) {
                vec2 off = vec2(float(x), float(y)) * texelsize;
                color += texture(texture0, fragTexCoord+off).xyz;
            }
        }

        color /= (3.0*3.0);
        
        float lines = sin(gl_FragCoord.y*0.5 + u_time*10.0) * 0.5 +0.5;
        lines *= lines;
        color *= (lines * vec3(0.1, 0.15, 0.1)+0.1) * 0.76;
    }

    finalColor.xyz = color;
    


    if(u_ssao_enabled == 1 && u_anygui_open == 0) {
        vec3 ao_col = color * texture(ssao_texture, fragTexCoord).rgb;
        finalColor = vec4(ao_col, 1.0);
    }

    finalColor.z += 0.025;
    // Gamma correction.
    finalColor.xyz = pow(finalColor.xyz, vec3(0.6));
}


