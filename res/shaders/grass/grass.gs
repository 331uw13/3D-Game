#version 430

layout (points) in;
layout (triangle_strip, max_vertices = 24) out;

in vec3 fragPosition[];
in mat4 viewproj[];

uniform float u_time;
uniform vec3  u_campos;
uniform float u_render_dist;


float rand(vec2 c){
	return fract(sin(dot(c.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

#define PI 3.14159

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

mat3 rotate_m3(vec2 ang) {
    vec2 c = cos(ang);
    vec2 s = sin(ang);
    return mat3(
            c.y,      0.0, -s.y,
            s.y*s.x,  c.x,  c.y*s.x,
            s.y*c.x, -s.x,  c.y*c.x);
}

void add_vertex(float x, float y, float z) {
    
    vec3 vert_pos = vec3(x, y, z);
    vec3 bpos = fragPosition[0];

    float pn = pNoise(fragPosition[0].xz, 1);

    //vec3 pos = fragPosition[0] + vec3(x, y, z);
   
    float random_rotation = bpos.x+bpos.z;

    mat3 m = rotate_m3(vec2(0.0, random_rotation));
    
    vec3 pos = bpos + vert_pos * m;

    // World space to clip space.
    gl_Position = viewproj[0] * (vec4(pos, 1.0));
    EmitVertex();
}

//#define ADD_VERTEX(x, y, z) gl_Position = gl_in[0].gl_Position + vec4(x, y, z, 0.0); EmitVertex()


void low_res_grassblade(float w, float h, float gh) {
    add_vertex(-w, -h, 0.0);  // Bottom Left
    add_vertex( w, -h, 0.0);  // Bottom Right
    add_vertex( w,  h, 0.0);  // Top Right
    
    // Triangle 2
    add_vertex( w,  h*gh, 0.0);  // Top Right
    add_vertex(-w, -h,    0.0);  // Bottom Left
    add_vertex(-w,  h*gh, 0.0);  // Top Left

    EndPrimitive();
}


void high_res_grassblade(float w, float h, float gh) {
 
    float q2_h = gh / 3.0;
    float q3_h = gh / 2.0;

    // Triangle 1
    add_vertex(-w, -h, 0.0);  // Bottom Left
    add_vertex( w, -h, 0.0);  // Bottom Right
    add_vertex( w,  h, 0.0);  // Top Right
    
    // Triangle 2
    add_vertex( w,  h, 0.0);  // Top Right
    add_vertex(-w, -h, 0.0);  // Bottom Left
    add_vertex(-w,  h, 0.0);  // Top Left
    

    // Triangle 3
    add_vertex(-w, h,    0.0);  // Bottom Left
    add_vertex( w, h,    0.0);  // Bottom Right
    add_vertex( w, h*q2_h,  0.0);  // Top Right
    
    // Triangle 4
    add_vertex( w,  h*q2_h, 0.0);  // Top Right
    add_vertex(-w,  h,   0.0);  // Bottom Left
    add_vertex(-w,  h*q2_h, 0.0);  // Top Left
    

    // Triangle 5
    add_vertex(-w,     h*q2_h,  0.0);  // Bottom Left
    add_vertex( w,     h*q2_h,  0.0);  // Bottom Right
    add_vertex( w*0.8, h*q3_h,  0.0);  // Top Right
    
    // Triangle 6
    add_vertex( w*0.8,  h*q3_h, 0.0);  // Top Right
    add_vertex(-w,      h*q2_h, 0.0);  // Bottom Left
    add_vertex(-w*0.8,  h*q3_h, 0.0);  // Top Left
    


    // Triangle 7
    add_vertex(-w*0.8, h*q3_h,  0.0);  // Bottom Left
    add_vertex( w*0.8, h*q3_h,  0.0);  // Bottom Right
    add_vertex( 0,     h*gh,  0.0);


    EndPrimitive();
}

float map(float t, float src_min, float src_max, float dst_min, float dst_max) {
    return (t - src_min) * (dst_max - dst_min) / (src_max - src_min) + dst_min;
}


void main() {

    float w = 0.2;  // Width
    float h = 1.0;  // Height

    float gh = 7.0; // Grass blade height

    float dist = length(fragPosition[0].xz - u_campos.xz);
    if(dist < 500.0) {
        high_res_grassblade(w, h, gh);
    }
    else {
        // Map width to distance because very small triangles are just waste of resources.
        float wmap = map(dist, 500.0, u_render_dist, 1.0, 4.0);
        wmap *= wmap;
        wmap = clamp(wmap, 1.0, 4.0);

        low_res_grassblade(w * wmap, h, gh);
    }



    EndPrimitive();
}


