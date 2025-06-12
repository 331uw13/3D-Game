
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

        float light_radius = 15.0;//lights[i].settings.x;
        float light_strength = 1.0;//lights[i].settings.y;

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
    
    g_lightcolor += max(dot(normal, sun_dir), 0.0) * (biome.sun_color.rgb*0.5);


    /*
    vec3 normal = normalize(fragNormal);
    for(int i = 0; i < MAX_NORMAL_LIGHTS; i++) {
        if(lights[i].enabled == 0) {
            continue;
        }
        vec3 lightpos = lights[i].pos.xyz;
        vec3 lightdir;
        float dist = 1.0;
        if(lights[i].type == LIGHT_POINT) {
            lightdir = normalize(lightpos - fragPosition);
            
            float light_radius = lights[i].strength.y;
            dist = distance(lights[i].pos.xyz, fragPosition) / light_radius;
            dist = 1.0/dist;
        }
        else
        if(lights[i].type == LIGHT_DIRECTIONAL) {
            lightdir = -normalize(-lightpos);
        }

        float NdotL = max(dot(normal, lightdir), 0.0);
        g_lightcolor += ((lights[i].color.rgb * NdotL)*dist) * lights[i].strength.x;
   
        float spec = 0.0;
        if(NdotL > 0.0) {
            spec = pow(max(0.0, dot(view_dir, reflect(-lightdir, normal))), 8.0);
        }

        if(lights[i].type != LIGHT_DIRECTIONAL) {
            g_lightspecular += (dist * lights[i].color.rgb) * (spec * spec);
        }
    }
    for(int i = 0; i < MAX_PROJECTILE_LIGHTS; i++) {
        if(prj_lights[i].enabled == 0) {
            continue;
        }
        vec3 lightpos = prj_lights[i].pos.xyz;
        vec3 lightdir;
        float dist = 6.0;
        lightdir = normalize(lightpos - fragPosition);
            
        float light_radius = prj_lights[i].strength.y;
        dist = distance(prj_lights[i].pos.xyz, fragPosition) / light_radius;
        dist = 1.0/dist;


        float NdotL = max(dot(normal, lightdir), 0.0);
        g_lightcolor += ((prj_lights[i].color.rgb * NdotL)*dist) * prj_lights[i].strength.x;
   
        float spec = 0.0;
        if(NdotL > 0.0) {
            spec = pow(max(0.0, dot(view_dir, reflect(-lightdir, normal))), 16.0);
            spec *= 1.865;
        }

        if(prj_lights[i].type != LIGHT_DIRECTIONAL) {
            g_lightspecular += (dist * prj_lights[i].color.rgb) * (spec * spec);
        }
   }
   */

}


