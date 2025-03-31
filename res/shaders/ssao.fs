#version 430

// IMPORTANT NOTE: This must be same as in 'src/state.h'
#define SSAO_KERNEL_SIZE 32

uniform sampler2D u_gbuf_pos_tex;
uniform sampler2D u_gbuf_norm_tex;
uniform sampler2D u_gbuf_depth_tex;
uniform sampler2D u_ssao_noise_tex;
uniform sampler2D u_bloomtresh_tex;
uniform vec3 ssao_kernel[SSAO_KERNEL_SIZE];
uniform mat4 u_camview_matrix;
uniform mat4 u_camproj_matrix;
uniform vec3 cam_pos;
uniform vec2 u_screen_size;

in vec2 fragTexCoord;
in vec4 fragColor;

out vec4 finalColor;


float ld(float depth) {
    float near = 0.01;     // NOTE: Remember to edit 'res/shaders/gbuffer.fs'
    float far = 3000.0;    //       if changing these values.
    float ndc = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - ndc * (far - near)) / far;
}

float map(float t, float src_min, float src_max, float dst_min, float dst_max) {
    return (t - src_min) * (dst_max - dst_min) / (src_max - src_min) + dst_min;
}



void main() {
    float ao = 0.0;


    // Very bright objects should not appear on top of ambient occlusion.
    // Convert bloom treshold texture to gray scale and check if ssao should be applied.

    vec3 bloomtresh = texture(u_bloomtresh_tex, fragTexCoord).rgb;
    float bloom_scale = (bloomtresh.r + bloomtresh.g + bloomtresh.b)/3.0;
    if(bloom_scale >= 0.076) {
        finalColor = vec4(1.0);
        return;
    }

    vec2 noise_scale = vec2(u_screen_size.x/8.0, u_screen_size.y/8.0);

    // Data from geometry buffer.
    vec3 frag_pos   = texture(u_gbuf_pos_tex, fragTexCoord).xyz;
    vec3 normal     = texture(u_gbuf_norm_tex, fragTexCoord).rgb;
    vec3 randomvec  = texture(u_ssao_noise_tex, fragTexCoord*noise_scale).xyz;
    mat4 viewproj   = u_camproj_matrix * u_camview_matrix;

    
    normal = normalize(normal);
    randomvec.xy = randomvec.xy * 2.0 - 1.0;
    randomvec.z = 0.0;

    vec3 tangent = normalize(randomvec - normal * dot(randomvec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    
    float depth = texture(u_gbuf_depth_tex, fragTexCoord).x;

    const float max_depth = 1200.0;

    // map radius, closer should be less effect, further away more.
    const float rad_near = 0.65;
    const float rad_far = 2.5;
    float dt = (viewproj*vec4(frag_pos,1.0)).z;

    if(dt > max_depth) {
        finalColor = vec4(1.0);
        return;
    }

    float tmax = 100.0;
    float radius = map(clamp(dt, 0.0, tmax), 0.0, tmax, rad_near, rad_far);


    for(int i = 0; i < SSAO_KERNEL_SIZE; i++) {
        vec3 sample_pos = TBN * ssao_kernel[i]; // Transform tangent space to view space.
        sample_pos = frag_pos + sample_pos * radius * 2.0;

        // View to clip space.
        vec4 offset = vec4(sample_pos, 1.0);
        offset = viewproj * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        offset = clamp(offset, vec4(0.0), vec4(1.0));
        float sample_depth = texture(u_gbuf_depth_tex, offset.xy).x;

        // Range check.
        float tr = ld(frag_pos.z);
        float rc = smoothstep(0.0, 1.0, radius/abs(tr - sample_depth));

        ao += ((sample_depth >= depth-ld(0.001)) ? 1.0 : 0.0) * rc;
    }

    ao /= float(SSAO_KERNEL_SIZE);
    
    // Fade ssao effect.
    float fade = (dt*dt)/(max_depth*max_depth);
    ao += fade;

    finalColor = vec4(ao, ao, ao, 1.0);
}


