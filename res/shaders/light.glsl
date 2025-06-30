
#define AMBIENT vec3(0.135, 0.165, 0.38)


// TODO: 'static_array_sizes' to include both in c and glsl.

// NOTE: Make sure this is same as in 'src/light.h'
#define MAX_LIGHTS 256

// 40 Bytes.
struct Light {
    vec4 color;
    vec4 position;
    vec4 settings;
};


layout(std430, binding = 2) buffer chunk_lights_buffer {
    Light lights[MAX_LIGHTS];
    int   num_lights;
};


#include "res/shaders/biome.glsl"


vec3 g_lightcolor = vec3(0, 0, 0);
vec3 g_lightspecular = vec3(0, 0, 0);



void compute_lights(vec3 view_dir) {
    
    vec3 normal = normalize(fragNormal);

   
    for(int i = 0; i < num_lights; i++) {

        vec3 light_pos = lights[i].position.xyz;
        vec3 light_dir = normalize(light_pos - fragPosition);
        vec3 view_dir = normalize(u_campos - fragPosition);
        vec3 halfway_dir = normalize(light_dir - view_dir);

        float light_radius = lights[i].settings.y;
        float light_strength = lights[i].settings.x;

        float dist = distance(light_pos, fragPosition) / light_radius;
        dist = 1.0 / dist;
   

        float NdotL = max(dot(normal, light_dir), 0.0);

        g_lightcolor += ((lights[i].color.rgb * NdotL) * dist) * light_strength;

        float spec = pow(max(dot(view_dir, reflect(-light_dir, normal)), 0.0), 8.0);
        g_lightspecular += (dist * lights[i].color.rgb) * spec;
    }
    


    // Add one directional light. It is the sun.

    vec3 sun_pos = vec3(0, 1, 0);
    vec3 sun_dir = -normalize(-sun_pos);
    
    g_lightcolor += max(dot(normal, sun_dir), 0.0) * (biome.sun_color.rgb*0.4);
}

