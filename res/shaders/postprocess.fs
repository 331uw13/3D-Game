
#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

#define SSAO_KERNEL_SAMPLE_SIZE 64  // NOTE: this must be same as in 'src/state.h'

// Input uniform values
uniform sampler2D texture0;
uniform sampler2D bloom_treshold_texture;
uniform sampler2D depth_texture;
uniform sampler2D ssao_texture;

uniform vec3 ssao_kernel_samples[SSAO_KERNEL_SAMPLE_SIZE];
uniform mat4 camera_proj;
uniform vec4 colDiffuse;
uniform float time;
uniform float health; // normalized
uniform vec2 screen_size;
uniform vec3 cam_target;
uniform vec3 cam_pos;
uniform mat4 sun_matrix;

// Output fragment color
out vec4 finalColor;

#define Pi 3.14159
#define Pi2 (Pi*2)
#define PiR (Pi/180.0)

float lerp(float t, float min, float max) {
    return (max - min) * t + min;
}


// Get color values above treshold.
vec3 bloom_treshold(vec3 fcolor, vec3 treshold) {
    vec3 result = vec3(0);

    float brightness = dot(fcolor, treshold);
    if(brightness > 1.0) {
        result = fcolor;
    }

    return result;
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
            result += BLOOM_ADD_M * texture(bloom_treshold_texture, fragTexCoord + p * sf).rgb;
        }     
    }


    for(int y = -r; y <= r; y++) {
        for(int x = -r; x <= r; x++) {
            vec2 p = vec2(x, y) * BLOOM_POS_M;
            result += BLOOM_ADD_M * texture(bloom_treshold_texture, fragTexCoord + p * sf).rgb;
        }     
    }


    return (result / BLOOM_SAMPLES) * colDiffuse.rgb;
}


#define SSAO_RADIUS 2.0
#define SSAO_BIAS 0.0025

vec2 saturate(vec2 x) {
    vec2 t;
    float a = x.x;
    float b = x.y;
    t.x = max(0, min(1, a));
    t.y = max(0, min(1, b));
    return t;
}


vec3 depthnormal(float depth)
{
    const vec2 offset1 = vec2(0.0f,0.001f);
    const vec2 offset2 = vec2(0.001f,0.0f);

    float depth1 = texture(depth_texture, fragTexCoord+offset1).r;
    float depth2 = texture(depth_texture, fragTexCoord+offset2).r;

    vec3 p1 = vec3(offset1, depth1 - depth);
    vec3 p2 = vec3(offset2, depth2 - depth);

    vec3 normal = cross(p1,p2);
    normal.z = -normal.z;

    return normalize(normal);
}

// Reference from: 
// * https://learnopengl.com/code_viewer_gh.php?code=src/5.advanced_lighting/9.ssao/9.ssao.fs
// * https://github.com/dotxnc/Raylib-GBuffers/blob/master/bin/assets/shaders/ssao.fs
//
// (this is probably not correct at all im not sure...)

/*
float get_ssao() {
    float result = 0.0;
    

    float depth = texture(depth_texture, fragTexCoord).r;

    vec3 normal     = normalize(texture(ssao_norm_tex, fragTexCoord).rgb);
    vec3 randomvec  = normalize(texture(ssao_noise_tex, fragTexCoord).rgb).xyz;
    float radius_depth = 0.5 / depth;


    vec3 fragpos = texture(ssao_pos_tex, fragTexCoord).xyz;

    vec3 tangent = normalize(randomvec - normal * dot(randomvec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    
    for(int i = 0; i < 64; i++) {

        vec3 sample_pos = TBN * ssao_kernel_samples[i];
        sample_pos = fragpos + sample_pos * 0.5;

        vec4 offset = vec4(sample_pos, 1.0);
        offset = camera_proj * offset;
        offset.xyz /= offset.w;
        offset.xyz = normalize(offset.xyz);

        vec3 ray = radius_depth * reflect(ssao_kernel_samples[i], randomvec);
        vec3 hemi_ray = vec3(offset.xy, depth) + sign(dot(ray, normal)) * ray;


        float occ_depth = texture(ssao_pos_tex, (hemi_ray.xy)).z;
       
        //float diff = depth - occ_depth;
        float range_check = smoothstep(0.0, 1.0, 0.5 / abs(fragpos.z - occ_depth));
        result += ((occ_depth >= sample_pos.z + 0.0024) ? 1.0 : 0.0) * range_check;
    }


    return result;
    //return max(0, min(1.0, ao));
}
*/
/*
float get_ssao() {
    float result = 0.0;
    

    float depth = texture(depth_texture, fragTexCoord).r;

    vec3 normal     = normalize(texture(ssao_norm_tex, fragTexCoord).rgb);
    vec3 randomvec  = normalize(texture(ssao_noise_tex, fragTexCoord).rgb).xyz;
    float radius_depth = 0.5 / depth;


    vec3 fragpos = texture(ssao_pos_tex, fragTexCoord).xyz;

    vec3 tangent = normalize(randomvec - normal * dot(randomvec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    
    for(int i = 0; i < 64; i++) {

        vec3 sample_pos = TBN * ssao_kernel_samples[i];
        sample_pos = fragpos + sample_pos * 0.5;

        vec4 offset = vec4(sample_pos, 1.0);
        offset = camera_proj * offset;
        offset.xyz /= offset.w;
        offset.xyz = normalize(offset.xyz);

        vec3 ray = radius_depth * reflect(ssao_kernel_samples[i], randomvec);
        vec3 hemi_ray = vec3(offset.xy, depth) + sign(dot(ray, normal)) * ray;


        float occ_depth = texture(ssao_pos_tex, (hemi_ray.xy)).z;
       
        //float diff = depth - occ_depth;
        float range_check = smoothstep(0.0, 1.0, 0.5 / abs(fragpos.z - occ_depth));
        result += ((occ_depth >= sample_pos.z + 0.0024) ? 1.0 : 0.0) * range_check;
    }


    return result;
    //return max(0, min(1.0, ao));
}
*/

vec3 blur_ssao() {
    vec3 result = vec3(0);
    
    vec2 texel_size = 1.0 / screen_size;

    for(int x = -2; x < 2; x++) {
        for(int y = -2; y < 2; y++) {
            vec2 offset = vec2(float(x), float(y)) * texel_size;
            result += texture(ssao_texture, fragTexCoord+offset).rgb;
        }
    }

    /*
    vec2 size = screen_size * 0.75;
    vec2 sf = 1.0/(size * 2.0);
    float blur_add = 0.85;
    int r = 2;
 
    for(int x = -r; x <= r; x++) {
        for(int y = -r; y <= r; y++) {
            vec2 p = vec2(x, y) * 0.9;
            result += blur_add * texture(ssao_texture, fragTexCoord + p * sf).rgb;
        }     
    }

    for(int y = -r; y <= r; y++) {
        for(int x = -r; x <= r; x++) {
            vec2 p = vec2(x, y) * 0.9;
            result += blur_add * texture(ssao_texture, fragTexCoord + p * sf).rgb;
        }     
    }
    */

    return result;
}


void main()
{


    vec3 color = texture(texture0, fragTexCoord).rgb;


    vec3 tobloom = texture(texture0, fragTexCoord).rgb;
    vec3 bloom = get_bloom();
    color += bloom;


    // Small blur effect to smooth things out.

    vec3 result = vec3(0);
    vec2 size = screen_size * 0.62;
    vec2 sf = 1.0/(size * 2.0);
    float blur_add = 0.6;
    int r = 3;
 
    for(int x = -r; x <= r; x++) {
        for(int y = -r; y <= r; y++) {
            vec2 p = vec2(x, y) * 0.9;
            result += blur_add * texture(texture0, fragTexCoord + p * sf).rgb;
        }     
    }

    for(int y = -r; y <= r; y++) {
        for(int x = -r; x <= r; x++) {
            vec2 p = vec2(x, y) * 0.9;
            result += blur_add * texture(texture0, fragTexCoord + p * sf).rgb;
        }     
    }

    color = mix(result/20.0, color, 0.5);

    // ---------------------
    
    finalColor = vec4(color, 1.0);

}




