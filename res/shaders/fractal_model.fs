
#version 430

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal_A;
in float base_ydist;
in float time;

uniform sampler2D texture0;
uniform vec3 u_campos;

out vec4 finalColor;

vec3 fragNormal = vec3(0);

#include "res/shaders/light.glsl"
#include "res/shaders/shadow.glsl"
#include "res/shaders/fog.glsl"


void main()
{
    vec3 view_dir = normalize(u_campos - fragPosition);
    vec3 dx = dFdx(fragPosition);
    vec3 dy = dFdy(fragPosition);

    fragNormal = normalize(cross(dx, dy));
    
    compute_lights(view_dir);




    vec3 ambient = fragColor.rgb * (AMBIENT/3.0);
    vec3 color = g_lightcolor * fragColor.rgb + ambient;
    
    float dist = length(u_campos - fragPosition);
    color = get_fog(color, dist, _YLEVEL);

    color *= get_shadows();

    finalColor = vec4(color, 1.0);


}
