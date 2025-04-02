
// Write bloom treshold values to texture and post process it later.

#version 430

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec2 u_screen_size;
uniform sampler2D u_gbuf_pos_tex;
uniform sampler2D u_gbuf_norm_tex;

// Output fragment color
out vec4 finalColor;



float map(float t, float src_min, float src_max, float dst_min, float dst_max) {
    return (t - src_min) * (dst_max - dst_min) / (src_max - src_min) + dst_min;
}


void main()
{
    vec3 res = vec3(0);
    


    int r = 2;
    vec2 ts = 1.0/u_screen_size;
    float sum = 0.0;


    // Current position.
    vec3 fragpos = texture(u_gbuf_pos_tex, fragTexCoord).xyz;
    vec3 normal = normalize(texture(u_gbuf_norm_tex, fragTexCoord).xyz)*2.0-1.0;


    for(int y = -r; y <= r; y++) {
        for(int x = -r; x <= r; x++) {
            vec2 offset = vec2(float(x), float(y)) * ts;
            vec2 sample_pos = fragTexCoord + offset;
            vec3 sample_color = texture(texture0, sample_pos).rgb;

            vec3 sample_fragpos = texture(u_gbuf_pos_tex, sample_pos).xyz;

            const float tresh = 0.5;
            if(abs((fragpos.z) - (sample_fragpos.z)) > tresh) {
                continue;
            }

            res += sample_color;
            sum += 1.0;

        }
    }
    
    finalColor.rgb = res / sum;
    finalColor.w = 1.0;


    r = 1;

    vec3 blur_result = vec3(0);
    for(int y = -r; y <= r; y++) {
        for(int x = -r; x <= r; x++) {
            vec2 offset = vec2(float(x), float(y)) * ts;
            vec2 sample_pos = fragTexCoord + offset * 1.01;
            vec3 sample_color = texture(texture0, sample_pos).rgb;

            blur_result += sample_color;
        }
    }

    finalColor.rgb += blur_result/16.0;


}



