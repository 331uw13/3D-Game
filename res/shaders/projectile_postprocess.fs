#version 330

// PROJECTILE POSTPROCESSING SHADER:


// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform sampler2D env_depth_tex;
uniform sampler2D prj_depth_tex;

uniform vec4 colDiffuse;
uniform float time;
uniform float health; // normalized
uniform vec2 screen_size;

// Output fragment color
out vec4 finalColor;

#define Pi 3.14159
#define Pi2 (Pi*2)

float lerp(float t, float min, float max) {
    return (max - min) * t + min;
}

void main()
{

    float env_d = texture(env_depth_tex, fragTexCoord).r;
    float prj_d = texture(prj_depth_tex, fragTexCoord).r;


    if(env_d < prj_d) {
        discard;
    }



    float gamma = 0.8;

    vec2 sf = 1.0 / screen_size * 4.0;


    vec3 texcolor = texture(texture0, fragTexCoord).rgb;
    finalColor = vec4(texcolor.rgb, 1.0);

    vec3 sum = vec3(0.0);
    
    const float pos_m = 0.3;
    const float add_m = 0.6;
    const float samples = 4.0;
    const int r = 3;

    for(int x = -r; x <= r; x++) {
        for(int y = -r; y <= r; y++) {
            vec2 p = vec2(x, y) * pos_m;
            sum += add_m * texture(texture0, fragTexCoord + p * sf).rgb;
        }
    }

    for(int y = -r; y <= r; y++) {
        for(int x = -r; x <= r; x++) {
            vec2 p = vec2(x, y) * pos_m;
            sum += add_m * texture(texture0, fragTexCoord + p * sf).rgb;
        }
    }



    finalColor.xyz = ((sum / (samples))+texcolor) * colDiffuse.rgb;


    float b = dot(finalColor.xyz, vec3(0.2126, 0.7152, 0.0722));
    finalColor.w = b * 0.3;


    //finalColor.xyz = texture(env_depth_tex, fragTexCoord).rgb;
}
