#version 430

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

uniform int u_chunk_grass_baseindex;
uniform float u_wind_strength;
uniform float u_time;
uniform vec3  u_wind_dir;
uniform int   u_num_forcevectors;
uniform vec2      u_chunk_coords;
uniform int       u_chunk_size;
//uniform vec2      u_terrain_origin;
uniform int     u_num_grass_perchunk;

layout (rgba8, binding = 0) uniform readonly image2D chunk_forcetex;

#include "res/shaders/grass/grassdata.glsl"

#define PI 3.14159


// Perlin noise from:
// https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
float rand(vec2 c){
	return fract(sin(dot(c.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
float noise(vec2 p, float freq){
	float unit = 200.0/freq;
	vec2 ij = floor(p/unit);
	vec2 xy = mod(p,unit)/unit;
	//xy = 3.*xy*xy-2.*xy*xy*xy;
	xy = .5*(1.-cos(PI*xy));
	float a = rand((ij+vec2(0.,0.)));
	float b = rand((ij+vec2(1.,0.)));
	float c = rand((ij+vec2(0.,1.)));
	float d = rand((ij+vec2(1.,1.)));
	float x1 = mix(a, b, xy.x);
	float x2 = mix(c, d, xy.x);
	return mix(x1, x2, xy.y);
}
float pNoise(vec2 p, int res){
	float persistance = .5;
	float n = 0.;
	float normK = 0.;
	float f = 4.;
	float amp = 1.;
	int iCount = 0;
	for (int i = 0; i<50; i++){
		n+=amp*noise(p, f);
		f*=2.;
		normK+=amp;
		amp*=persistance;
		if (iCount == res) break;
		iCount++;
	}
	float nf = n/normK;
	return nf*nf*nf*nf;
}
// ------


mat3 rotate_m3(vec2 ang) {
    vec2 c = cos(ang);
    vec2 s = sin(ang);
    return mat3(
            c.y,      0.0, -s.y,
            s.y*s.x,  c.x,  c.y*s.x,
            s.y*c.x, -s.x,  c.y*c.x);
}



/*
#define MAX_GRASS_FORCEVECTORS 16



layout (std140, binding = 6) uniform force_vec_ub {
    // XYZ = Position
    // W = Strength
    vec4 force_vectors[MAX_GRASS_FORCEVECTORS];
};
*/

mat3 rotate_to_dir(vec3 dir) {
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 a = normalize(dir);
    vec3 b = normalize(cross(a, up));
    vec3 c = normalize(cross(b, a));
    return mat3(a, b, c);
}

#define PI 3.14159


float map(float t, float src_min, float src_max, float dst_min, float dst_max) {
    return (t - src_min) * (dst_max - dst_min) / (src_max - src_min) + dst_min;
}


// How far away Y axis will start to scale radius?
#define MAX_YSCALE_DIST  45.0

void main() {
    
    uint id = gl_GlobalInvocationID.x + uint(u_chunk_grass_baseindex);

    vec2 noisepos = vec2(
            grassdata[id].position.x,
            grassdata[id].position.z
            );


    // Calculate rotation.

    mat3 rotation = rotate_to_dir(u_wind_dir);

    float shift = pNoise(grassdata[id].position.xz*2.0, 1)*1.5;
    rotation *= rotate_m3(vec2(1.5, shift));
    rotation *= rotate_m3(vec2(0.0, 1.5));


    float num = float(u_num_grass_perchunk);
    float sqrt_chunk_size = sqrt(u_chunk_size);
    float scale_factor = num / sqrt_chunk_size / (u_chunk_size*4);
    vec2 localcoord = 
        (grassdata[id].position.xz - u_chunk_coords)
        / (u_chunk_size
        / (((num / sqrt_chunk_size) / (u_chunk_size * scale_factor)) + 0.25)
        );

    localcoord = clamp(localcoord, vec2(0.0), vec2(u_chunk_size));


    //grassdata[id].ext_force = imageLoad(chunk_forcetex, ivec2(localcoord));
 



    grassdata[id].rotation = mat3x4(rotation);
    
    noisepos -= u_wind_strength * vec2(u_wind_dir.x * u_time, u_wind_dir.z * u_time);
    float pn = pNoise(noisepos*0.2, 1) * 1.235;

    GRASSDATA_BEND_VALUE(id) = pn + 0.1 ;
}



