
#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;
// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;


// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

out float base_ydist;
out float time;

uniform float u_fractal_base_y;
uniform float u_time;

void main()
{
    vec3 vertpos = vertexPosition;

    float y = vec3(matModel*vec4(vertpos, 1.0)).y;
    float yd = u_fractal_base_y - y;
    
    base_ydist = yd;
    time = u_time;

    vertpos.x += ((sin(u_time*2.251) * (sin(yd*0.5+u_time)*0.162)) * yd) * 0.3;
    vertpos.z += ((sin(u_time*2.852) * (cos(yd*0.5+u_time)*0.174)) * yd) * 0.3;

    // Send vertex attributes to fragment shader
    fragPosition = vec3(matModel*vec4(vertpos, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = normalize(vec3(matNormal*vec4(vertexNormal, 1.0)));

    // Calculate final vertex position
    gl_Position = mvp*vec4(vertpos, 1.0);
    
}
