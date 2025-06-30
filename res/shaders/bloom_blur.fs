
#version 430

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec2 u_screen_size;


out vec4 finalColor;

#include "res/shaders/voronoi.glsl"


void main()
{
    vec3 color = vec3(0.0, 0.0, 0.0);

    vec2 texelsize = 1.0/(u_screen_size);


    int size = 26;
    float weight_sum = 0;

    vec2 uv = gl_FragCoord.xy / u_screen_size;

    for(int x = -size; x <= size; x++) {
        for(int y = -size; y <= size; y++) {
            vec2 off = vec2(float(x), float(y));

            float len = length(off);// * 1.2;
            float weight = float(size+3.0) - len;
           
            vec2 texelpos = fragTexCoord + off * texelsize;
            
            if((texelpos.y >= 1.0) || (texelpos.y <= 0.0)
            || (texelpos.x >= 1.0) || (texelpos.x <= 0.0)) {
                continue;
            }
            vec3 p = texture(texture0, texelpos).rgb;

            color += weight * p;
            weight_sum += weight;
        }
    }

    color /= (weight_sum*0.4);


    finalColor = vec4(color, clamp(length(color), 0.0, 1.0));
}








