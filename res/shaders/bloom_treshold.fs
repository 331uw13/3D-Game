
// Write bloom treshold values to texture and post process it later.

#version 430

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;



// Get color values above treshold.
vec3 bloom_treshold(vec3 fcolor, vec3 treshold) {
    vec3 result = vec3(0);

    float brightness = dot(fcolor, treshold);
    if(brightness > 1.0) {
        result = fcolor;
    }

    return result;
}


void main()
{
    vec3 color = texture(texture0, fragTexCoord).rgb * 1.5;
    color = bloom_treshold(color, vec3(1.0, 1.0, 1.0));



    finalColor = vec4(color * 1.25, 1.0);
}
