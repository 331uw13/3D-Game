
#version 430


// IMPORTANT NOTE: This must be same as in 'src/state.h'
#define SSAO_KERNEL_SIZE 64  


// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D gbuf_pos_tex;
uniform sampler2D gbuf_norm_tex;
uniform sampler2D gbuf_difspec_tex; // (NOT CURRENTLY USED)
uniform sampler2D gbuf_depth;

uniform sampler2D ssao_noise_tex;
uniform vec3 ssao_kernel[SSAO_KERNEL_SIZE];
uniform mat4 cam_view;
uniform mat4 cam_proj;

uniform sampler2D texture0;
uniform sampler2D bloomtresh_texture;

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
    vec2 size = screen_size * 0.22;
    vec2 sf = 1.0/(size * 2.0);
    const int r = 4;

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

float ld(float depth) {
    float near = 0.1;
    float far = 100.0;
    return near * far / (far + depth * (near - far));
}

float get_ssao() {

    vec2 noise_scale = vec2(screen_size.x/4.0, screen_size.y/4.0);

    vec3 frag_pos   = texture(gbuf_pos_tex, fragTexCoord).xyz;
    vec3 normal     = texture(gbuf_norm_tex, fragTexCoord).rgb;
    vec3 randomvec  = texture(ssao_noise_tex, fragTexCoord*noise_scale).xyz;


    normal = normalize(normal);
    randomvec.xy = randomvec.xy * 2.0 - 1.0;

    // Get TBN matrix to transform from 'tangent space' to 'view space'
    vec3 tangent = normalize(randomvec - normal * dot(randomvec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);


    mat4 viewproj = cam_proj * cam_view;
    normal = (viewproj * vec4(normal * cam_pos, 1.0)).xyz;


    const float radius = 2.0;
    const float bias = 0.0025;

    float ao = 0.0;

    for(int i = 0; i < SSAO_KERNEL_SIZE; i++) {

        vec3 sample_pos = TBN * ssao_kernel[i];
        sample_pos = frag_pos + sample_pos * radius;


        vec4 offset = vec4(sample_pos, 1.0);
        offset = viewproj * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz*0.5+0.5;


        vec3 sample_pos_vp = (viewproj * vec4(sample_pos, 1.0)).xyz;
        vec3 test_pos_vp = (viewproj * vec4(texture(gbuf_pos_tex, offset.xy).xyz, 1.0)).xyz;        

        float depth = sample_pos_vp.z;
        float test_depth = test_pos_vp.z;

        float range_check = smoothstep(0.0, 1.0, radius / abs(sample_pos_vp.z - test_pos_vp.z));
        ao += ((depth <= test_depth) ? 1.0 : 0.0) * range_check;

    }


    ao /= float(SSAO_KERNEL_SIZE);

    return ao * ao;
}



void main()
{

    /*
    float ao = get_ssao();
    finalColor.rgb = vec3(ao, ao, ao);
    finalColor.w = 1.0;
    return;
*/


    vec2 texcoords = fragTexCoord;
    if(blur_effect < 0.5) {
        texcoords.x += (sin(gl_FragCoord.y*0.8+time*30.0)*0.5+0.5)*0.5;
    }

    vec3 color = texture(texture0, texcoords).rgb;

    //vec3 tobloom = texture(texture0, fragTexCoord).rgb;
    vec3 bloom = get_bloom();
    color += bloom;


    // Small blur effect to smooth things out.

    vec3 result = vec3(0);


    vec2 size = screen_size * blur_effect;
    vec2 sf = 1.0/(size * 2.0);
    float blur_add = 0.8;
    int r = 3;
 
    for(int x = -r; x <= r; x++) {
        for(int y = -r; y <= r; y++) {
            vec2 p = vec2(x, y) * 0.7;
            result += blur_add * texture(texture0, fragTexCoord + p * sf).rgb;
        }     
    }

    for(int y = -r; y <= r; y++) {
        for(int x = -r; x <= r; x++) {
            vec2 p = vec2(x, y) * 0.7;
            result += blur_add * texture(texture0, fragTexCoord + p * sf).rgb;
        }     
    }

    color = mix(result/20.0, color, 0.5);

    // ---------------------
    
    finalColor = vec4(color * get_ssao(), 1.0);
}




