
// Write bloom treshold values to texture and post process it later.

#version 430

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec2 u_screen_size;

// Output fragment color
out vec4 finalColor;



void main()
{
    vec3 color = vec3(0.0, 0.0, 0.0);

    vec2 texelsize = 1.0/(u_screen_size);



    int size = 15;
    float weight_sum = 0.0;

    for(int x = -size; x <= size; x++) {
        for(int y = -size; y <= size; y++) {
            vec2 off = vec2(float(x), float(y));
            float weight = float(size+2.0)-1.2*length(off);
            
            vec2 texelpos = fragTexCoord + off * texelsize;
            texelpos = clamp(texelpos, vec2(0.0), vec2(1.0));
            color += weight * texture(texture0, texelpos).rgb;
        
            weight_sum += weight;
        }
    }

    color /= (float(size)*float(size))*4.75;


    finalColor = vec4(color, clamp(length(color), 0.0, 1.0));
}
