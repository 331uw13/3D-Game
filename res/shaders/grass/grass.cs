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

void main() {
    
    uint id = gl_GlobalInvocationID.x + uint(u_chunk_grass_baseindex);

    vec2 noisepos = vec2(
            grassdata[id].position.x,
            grassdata[id].position.z
            );


    // Calculate rotation.


    // First rotate towards the wind direction.
    /*
    vec3 windno_y = vec3(u_wind_dir.x, 0, u_wind_dir.z);
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 wx = normalize(u_wind_dir);
    vec3 wy = normalize(cross(wx, up));
    vec3 wz = normalize(cross(wy, wx));
    */
    mat3 rotation = rotate_to_dir(u_wind_dir);//mat3(wx, wy, wz);

    float shift = pNoise(grassdata[id].position.xz*2.0, 1)*1.5;
    rotation *= rotate_m3(vec2(1.5, shift));
    rotation *= rotate_m3(vec2(0.0, 1.5));


    // ?????? has to be easier way to do this.. dont better solution yet.
    float num = float(u_num_grass_perchunk);
    float huh = num / sqrt(u_chunk_size) / (u_chunk_size*4);
    vec2 localcoord = 
        (grassdata[id].position.xz - u_chunk_coords)
        / (u_chunk_size
        / (((num / sqrt(u_chunk_size)) / (u_chunk_size * huh)) + 0.25)
        );

        /// (u_chunk_size / (u_grass_spacing / 2.0 - 0.1));

    localcoord = clamp(localcoord, vec2(0.0), vec2(u_chunk_size));
    vec3 fvec = imageLoad(chunk_forcetex, ivec2(localcoord)).xyz;



    //grassdata[id].settings.y = fvec.z * 10;

    //vec3 fvec = texture(u_chunk_forcetex, vec2(0.5, 0.5)).rgb;
    //grassdata[id].settings.y = fvec.x;


    // Add force vectors.
    /*
    for(int i = 0; i < u_num_forcevectors; i++) {
        if(force_vectors[i].w > 0.0) {

            vec3 dir = grassdata[id].position.xyz - force_vectors[i].xyz;
            float len = length(dir); // Distance.

            len = clamp(len, 0.0, 50.0);
            len = 1.0-(len/50.0);

            vec3 diff = force_vectors[i].xyz - grassdata[id].position.xyz;
            float dz = -(atan(diff.x, diff.z))*len;

            mat3 rot =  rotate_m3(vec2(1.5*len, 0.0));
                 rot *= rotate_m3(vec2(0.0, dz));

            rotation *= rot;

        }
    }
    */
    
    grassdata[id].rotation = mat3x4(rotation);


    noisepos -= u_wind_strength * vec2(u_wind_dir.x * u_time, u_wind_dir.z * u_time);
    float pn = pNoise(noisepos*0.2, 1) * 1.235;

    GRASSDATA_BEND_VALUE(id) = pn + 0.1;
}



