
#version 430


// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform sampler2D bloomtresh_texture;
uniform sampler2D ssao_texture;
uniform sampler2D depth_texture;

uniform vec4 colDiffuse;
uniform float time;
uniform float health; // normalized
uniform float blur_effect;
uniform vec2 screen_size;
uniform vec3 cam_target;
uniform vec3 cam_pos;

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

vec3 get_bloom() {
    vec3 result = vec3(0);
    vec2 size = screen_size * 0.365;
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
}




void main()
{
    //finalColor = texture(ssao_texture, fragTexCoord); return;


    vec2 texcoords = fragTexCoord;
    if(blur_effect < 0.5) {
        texcoords.x += (sin(gl_FragCoord.y*0.8+time*30.0)*0.5+0.5)*0.5;
    }

    vec3 color = texture(texture0, texcoords).rgb;

    vec3 bloom = get_bloom();
    color += bloom;


    vec3 ao_col = color * texture(ssao_texture, fragTexCoord).rgb;
    finalColor = vec4(ao_col, 1.0);

    // Gamma correction.
    finalColor.xyz = pow(finalColor.xyz, vec3(0.6));
}


