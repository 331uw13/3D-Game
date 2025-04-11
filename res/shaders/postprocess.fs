
#version 430


// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform sampler2D u_bloomtresh_tex;
uniform sampler2D ssao_texture;

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




vec3 color_lerp(float t, vec3 a, vec3 b) {
    return vec3(
            lerp(t, a.r, b.r),
            lerp(t, a.g, b.g),
            lerp(t, a.b, b.b)
            );
}



void main()
{
//    finalColor = texture(ssao_texture, fragTexCoord); return;

    finalColor = vec4(0.0, 0.0, 0.0, 1.0);
    

    vec4 color = texture(texture0, fragTexCoord);
    color.rgb += texture(u_bloomtresh_tex, fragTexCoord).rgb;
 
    if(u_anygui_open == 1) {
        vec2 texelsize = 1.0/(u_screen_size*0.5);
        const int r = 3;
        for(int x = -r; x <= r; x++) {
            for(int y = -r; y <= r; y++) {
                vec2 off = vec2(float(x), float(y)) * texelsize;
                color.rgb += texture(texture0, fragTexCoord+off).xyz;
            }
        }

        color.rgb /= (3.0*3.0);
        
        float lines = sin(gl_FragCoord.y*0.5 + u_time*10.0) * 0.5 +0.5;
        lines *= lines;
        color.rgb *= (lines * vec3(0.1, 0.15, 0.1)+0.1) * 0.76;
    }

    color.rgb = pow(color.rgb, vec3(0.6));
    if(u_ssao_enabled == 1 && u_anygui_open == 0) {
        color.rgb *= texture(ssao_texture, fragTexCoord).rgb;
    }
    finalColor = color;

}


