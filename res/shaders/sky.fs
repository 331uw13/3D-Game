#version 330

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Input uniform values
uniform sampler2D texture0;
uniform vec3 sun_position;
uniform vec3 view_position;
uniform float time;

// Output fragment color
out vec4 finalColor;



// ###########################################################

// https://github.com/MaxBittker/glsl-voronoi-noise/tree/master

const mat2 myt = mat2(.12121212, .13131313, -.13131313, .12121212);
const vec2 mys = vec2(1e4, 1e6);

vec2 rhash(vec2 uv) {
  uv *= myt;
  uv *= mys;
  return fract(fract(uv / mys) * uv);
}

vec3 hash(vec3 p) {
  return fract(
      sin(vec3(dot(p, vec3(1.0, 57.0, 113.0)), dot(p, vec3(57.0, 113.0, 1.0)),
               dot(p, vec3(113.0, 1.0, 57.0)))) *
      43758.5453);
}

vec3 voronoi3d(const in vec3 x) {
  vec3 p = floor(x);
  vec3 f = fract(x);

  float id = 0.0;
  vec2 res = vec2(10.0);
  for (int k = -1; k <= 1; k++) {
    for (int j = -1; j <= 1; j++) {
      for (int i = -1; i <= 1; i++) {
        vec3 b = vec3(float(i), float(j), float(k));
        vec3 r = vec3(b) - f + hash(p + b);
        float d = dot(r, r);

        float cond = max(sign(res.x - d), 0.0);
        float nCond = 1.0 - cond;

        float cond2 = nCond * max(sign(res.y - d), 0.0);
        float nCond2 = 1.0 - cond2;

        id = (dot(p + b, vec3(1.0, 57.0, 113.0)) * cond) + (id * nCond);
        res = vec2(d, res.x) * cond + res * nCond;

        res.y = cond2 * d + nCond2 * res.y;
      }
    }
  }

  return vec3(sqrt(res), abs(id));
}

// ###########################################################


void main()
{
    vec3 point = (fragPosition - view_position);

    vec3 col = vec3(0.0);
    vec3 sun_color = vec3(1.0, 0.3, 0.1);
    vec3 horizon_color = vec3(0.0, 0.1, 0.1);


    vec3 vpos = sun_position - point;

    vec3 voronoi = voronoi3d(vpos*5 - sun_position*time);
    voronoi.x = voronoi.x+0.5;
   

    float sundst_plu =     length(point - sun_position);
    float sundst_inv = 1.0/sundst_plu;
    sundst_inv *= sundst_inv;
    sundst_inv *= 0.03;

    sundst_plu = 1.0-sundst_plu;
    sundst_plu = pow(sundst_plu, 5);
    sundst_plu *= sundst_plu;


    //sun_color.x += sin(time)*0.5+0.5;

    sun_color += (voronoi.x * sundst_plu);

    col = sun_color * sundst_inv;

    finalColor = vec4(col, 1.0);
}





