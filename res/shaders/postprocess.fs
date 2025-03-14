
#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform sampler2D bloom_treshold_texture;
uniform sampler2D depth_texture;

uniform vec4 colDiffuse;
uniform float time;
uniform float health; // normalized
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


// Get color values above treshold.
vec3 bloom_treshold(vec3 fcolor, vec3 treshold) {
    vec3 result = vec3(0);

    float brightness = dot(fcolor, treshold);
    if(brightness > 1.0) {
        result = fcolor;
    }

    return result;
}

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




void main()
{
    vec3 color = texture(texture0, fragTexCoord).rgb;


    vec3 tobloom = texture(texture0, fragTexCoord).rgb;
    vec3 bloom = get_bloom();
    color += bloom;


    // small blur effect to smooth things out.

    vec3 result = vec3(0);
    vec2 size = screen_size * 0.62;
    vec2 sf = 1.0/(size * 2.0);
    float blur_add = 0.6;
    const int r = 3;
    
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

    if(cam_pos.y < -80.0) {
        color += vec3(0.0, 0.3, 0.3);
    }


    finalColor = vec4(color, 1.0);
}



