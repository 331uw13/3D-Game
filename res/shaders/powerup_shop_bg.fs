
#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Output fragment color
out vec4 finalColor;


uniform float time;
uniform vec2 screen_size;

#define PI 3.14159
#define PI2 (PI*2)
#define PIR (PI/180.0)

#define FOV 80.0
#define MAX_RAYLEN 60.0
#define MIN_DIST 0.1


// TODO: Render in smaller resolution and upscale.


float lerp(float t, float min, float max) {
    return (max - min) * t + min;
}

vec3 raydir() {
    vec2 rs = screen_size*0.5;
    float hf = tan((90.0-FOV*0.5)*(PIR));
    return normalize(vec3(gl_FragCoord.xy-rs, (rs.y*hf)));
}

// https://iquilezles.org/articles/distfunctions/
// big help from ^

float sphere_sdf(vec3 p, float r) {
    return length(p)-r;
}
float boxframe_sdf( vec3 p, vec3 b, float e ) {
       p = abs(p  )-b;
  vec3 q = abs(p+e)-e;
  return min(min(
      length(max(vec3(p.x,q.y,q.z),0.0))+min(max(p.x,max(q.y,q.z)),0.0),
      length(max(vec3(q.x,p.y,q.z),0.0))+min(max(q.x,max(p.y,q.z)),0.0)),
      length(max(vec3(q.x,q.y,p.z),0.0))+min(max(q.x,max(q.y,p.z)),0.0));
}

mat3 rotm3(vec2 ang) {
    vec2 c = cos(ang);
    vec2 s = sin(ang);
    return mat3(c.y, 0.0, -s.y, s.y*s.x, c.x, c.y*s.x, s.y*c.x, -s.x, c.y*c.x);
}

mat2 rotm2(float a) {
    float c = cos(a);
    float s = sin(a);
    return mat2(c, -s, s, c);
}

vec3 replim(vec3 p, float s, float lim) {
    return p - s * clamp(round(p / s), -lim, lim);
}

vec3 rep(vec3 p, vec3 s) {
    return p - s * round(p/s);
}

vec4 map(vec3 p);

#define RGB_PALETTE vec3(0.5,0.5,0.5), vec3(0.5,0.5,0.5), vec3(1.0,1.0,1.0), vec3(0.0,0.33,0.67)
#define SOFT_RGB_PALETTE vec3(0.5,0.5,0.5), vec3(0.5,0.5,0.5), vec3(2.0,1.0,0.0), vec3(0.50,0.20,0.25)
#define COLOR_PALETTE vec3(0.8, 0.5, 0.4), vec3(0.2, 0.4, 0.2), vec3(2.0, 1.0, 1.0), vec3(0.0, 0.25, 0.25)

// https://iquilezles.org/articles/palettes/
vec3 palette(float t, vec3 a, vec3 b, vec3 c, vec3 d) {
    return a + b*cos( 6.283185*(c*t+d) );
}

vec3 compute_normal(vec3 p) {
    vec2 e = vec2(0.001, 0.0);
    return normalize(vec3(
        map(p-e.xyy).w,
        map(p-e.yxy).w,
        map(p-e.yyx).w
    ));
}

vec3 compute_light(vec3 p, vec3 ro, vec3 light_pos, vec3 light_color) {
    vec3 ambient = light_color * 0.25;
    vec3 diffuse = light_color;

    vec3 light_dir = normalize(light_pos - p);

    vec3 n = compute_normal(p);
    return ambient+diffuse*max(0.0, dot(light_dir, n));

}

vec4 min_sdf(vec4 a, vec4 b) {
    return (a.w < b.w) ? a : b;
}

#include "res/shaders/voronoi.glsl"

vec4 map(vec3 p) {
    vec4 ret = vec4(0.0, 0.0, 0.0, 999999.99);


    {
        vec3 q = p;

        q.xy *= rotm2(sin(p.z*0.008)+time*0.2);
        q = rep(q, vec3(15.0, 15.0, 30.0));
        
        q = abs(q) - sin(0.89+time)*0.5+0.5;
        q.zy *= rotm2(time);
        q.xz *= rotm2(time);

        vec3 s0col = vec3(0.5, 0.5, 0.5);
        vec4 s0 = vec4(s0col, boxframe_sdf(q, vec3(5.0), 0.15) );
        ret = min_sdf(ret, s0);
    }




    return ret;
}
// xyz = Color, w = Distance.
//
/*
vec4 map(vec3 p) {
    vec4 ret = vec4(0.0, 0.0, 0.0, 999999.99);


    {
        vec3 q = p;

        q.xy *= rotm2(sin(p.z*0.002)+time*0.2);

        q = rep(q, vec3(10.0, 10.0, 30.0));
        q = abs(q) - sin(p.z*0.35) * 5.0+1.0;

        vec3 s0col = vec3(0.5, 0.5, 0.5);
        vec4 s0 = vec4(s0col, boxframe_sdf(q, vec3(5.0), 0.15));
        ret = min_sdf(ret, s0);
    }


    return ret;
}
*/

vec4 raymarch(vec3 ro, vec3 rd) {
    vec3 color = vec3(0.0, 0.0, 0.0);

    float rl = 0.01;
    vec3 p = vec3(0.0, 0.0, 0.0);

    int was_hit = 0;
    float closest_hit = 0.0;
    float steps = 0.0;

    for(; rl < MAX_RAYLEN; ) {
        p = ro+rd*rl;
        vec4 closest = map(p);

        if(closest.w < MIN_DIST) {
            vec3 light_pos = vec3(3.0, -50.0, 20.0);
            vec3 light_color = vec3(1.0, 1.0, 1.0);
            color = closest.xyz * compute_light(p, ro, light_pos, light_color);

            float pulse = sin(time*10.0 + p.z*0.1)*0.5+0.5;


            was_hit = 1;
            break;
        }

        if(closest_hit < closest.w) {
            closest_hit = closest.w;
        }

        steps += 1.0;
        rl += max(closest.w, 0.0001);
    }


    if(was_hit == 0) {
        steps *= steps * steps;
        float glow = steps / (MAX_RAYLEN * MAX_RAYLEN * MAX_RAYLEN);
    
        glow = pow(glow, 0.15);

        float vnoise = voronoi3d(p*0.2).x;
        vec3 glow_color = palette(vnoise+sin(p.z*0.2+time), RGB_PALETTE);
        color += glow * glow_color;
    }

    return vec4(color, rl);
}


void main() {

    vec3 color = vec3(0.0, 0.0, 0.0);

    vec3 ro = vec3(0, 0, time*8.0);
    vec3 rd = raydir();

    color = raymarch(ro, rd).xyz;


    finalColor.w = 0.15;
    finalColor.xyz = color;
}





