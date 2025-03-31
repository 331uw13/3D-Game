

// NOTE: these 2 values must same as in 'src/light.h'
#define MAX_PROJECTILE_LIGHTS  128
#define MAX_NORMAL_LIGHTS      16


#define LIGHT_DIRECTIONAL   0
#define LIGHT_POINT         1

#define AMBIENT vec3(0.125, 0.155, 0.35)


struct Light {
                   //  Base alignment  |  Aligned offset
    int type;      //  4                  0
    int enabled;   //  4                  16
    vec4 color;    //  16                 32
    vec4 pos;      //  16                 48
    vec4 strength; //  16                 64

};

// "Normal" lights
layout (std140, binding = 2) uniform light_ub {
    Light lights[MAX_NORMAL_LIGHTS];
};

// Projectile lights
layout (std140, binding = 3) uniform prj_light_ub {
    Light prj_lights[MAX_PROJECTILE_LIGHTS];
};


vec3 g_lightcolor = vec3(0);
vec3 g_lightspecular = vec3(0);



void compute_lights(vec3 view_dir) {
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

}


