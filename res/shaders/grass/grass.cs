#version 430

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

uniform int u_chunk_grass_baseindex;
uniform float u_wind_strength;
uniform float u_time;
uniform vec3  u_wind_dir;

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



void main() {
    
    uint id = gl_GlobalInvocationID.x + uint(u_chunk_grass_baseindex);

    vec2 noisepos = vec2(
            grassdata[id].position.x,
            grassdata[id].position.z
            );

    // TODO: this is wrong. but for test its ok.
    noisepos -= u_wind_strength * (u_wind_dir.xz + u_time);

    float pn = pNoise(noisepos*0.2, 1);

    GRASSDATA_BEND_VALUE(id) = pn * 3.0;

}

