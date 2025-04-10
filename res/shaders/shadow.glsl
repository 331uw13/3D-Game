
#define MAX_SHADOW_LEVELS 3

uniform sampler2D shadow_maps[MAX_SHADOW_LEVELS];
uniform mat4      u_shadowview_matrix[MAX_SHADOW_LEVELS];
uniform mat4      u_shadowproj_matrix[MAX_SHADOW_LEVELS];
uniform float     u_shadow_fov[MAX_SHADOW_LEVELS];
uniform float     u_shadow_bias;
uniform vec2      u_shadow_res;

vec2 prev_frustrum = vec2(0);

vec3 get_shadow_map(int idx) {
    vec3 shadows = vec3(1.0);

    mat4 shvp = u_shadowproj_matrix[idx] * u_shadowview_matrix[idx];
    vec4 shadow_space = shvp * vec4(fragPosition, 1.0);
    // Perspective divide Not needed for orthographic projection but will keep it here.
    vec3 dcoords = shadow_space.xyz / shadow_space.w;


    // Make sure it is in shadow camera frustrum.
    if(dcoords.x >= -1.0 && dcoords.x <= 1.0
    && dcoords.y >= -1.0 && dcoords.y <= 1.0) {
        shadows = vec3(0.0);
        int samples = 3;
        vec2 ts = 1.0/u_shadow_res;
        
        for(int x = -samples; x <= samples; x++) {
            for(int y = -samples; y <= samples; y++) {
                vec2 off = ts * vec2(float(x), float(y));

                // Normalize to 0.0 - 1.0.
                vec2 tfcoords = off+(dcoords.xy+1.0)/2.0;
                
                // Closest position from shadow camera perspective.
                vec3 closest = texture(shadow_maps[idx], tfcoords).xyz;

                if((closest.y - u_shadow_bias) < fragPosition.y) {
                    shadows += 0.5;
                }
            }
        }

        shadows /= (float(samples)*float(samples));
        shadows += vec3(0.2, 0.5, 1.0);

        //shadows += dist;
        
        shadows = clamp(shadows, vec3(0.0), vec3(1.0));
    }
    else {
        // For debug.
        //shadows = vec3(0.9, 0.9, 0.9);
    }

    prev_frustrum = dcoords.xy;
    return shadows;
}

// 'sfov' = shadow cam fov.
// 'psfov' = previous added shadow cam fov.
// NOTE: Calculating shadow cam fov as its distance maybe doesnt work very well with perspective projection(??)

float get_shadow_area(float sfov, float psfov) {
    float dist = length(fragPosition.xz - u_campos.xz);
    float s0 = step(dist, sfov/3.0);
    if(psfov > 0.0) {
        float s1 = 1.0-step(dist, psfov/3.0);
        s0 *= s1;
    }

    return s0;
}

vec3 get_shadows() {

    vec3 shadows = vec3(0);

    // First level area.
    float area0 = get_shadow_area(u_shadow_fov[0], -1.0);
    
    // Second level area.
    float area1 = get_shadow_area(u_shadow_fov[1], u_shadow_fov[0]);
    
    // Third level area.
    float area2 = get_shadow_area(u_shadow_fov[2], u_shadow_fov[1]);
   
    
    vec3 map0 = get_shadow_map(0) * area0;
    vec3 map1 = get_shadow_map(1) * area1;
    vec3 map2 = get_shadow_map(2) * area2;

    shadows += map0 + map1 + map2;

    return shadows;
}

