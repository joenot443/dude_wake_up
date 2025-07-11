#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

uniform vec2 mouse;

// New Uniforms
uniform float u_warp;
uniform float u_sunSize;
uniform vec3 u_sunColor;
uniform float u_sunSpread;
uniform float u_starCount;
uniform float u_starScale;
uniform float u_starBrightness;


#define time time
#define resolution dimensions.xy
#define PI2 6.28318530718
#define MAX_ITER 5


float snow(vec2 uv,float scale)
{
  float w=smoothstep(1.,0.,-uv.y*(scale/10.));if(w<.0)return 0.;
  
  uv*=scale;vec2 s=floor(uv),f=fract(uv),p;float k=40.,d;
  p=.5+.35*sin(11.*fract(sin((s+p+scale)*mat2(7,3,6,5))*5.))-f;d=length(p);k=min(d,k);
  k=smoothstep(0.,k,sin(f.x+f.y)*0.003);
  return k*w;
}
#define MAX_DISTANCE 1.0e+10

struct Plane {
  vec3 origin;
  vec3 normal;
  vec3 color;
};

struct Sphere {
  float radius;
  vec3 origin;
  vec3 color;
};

struct AABox {
  vec3 min;
  vec3 max;
  vec3 color;
};

struct Intersection {
  float t;
  vec3 point;
  vec3 normal;
  vec3 color;
};

// from: https://gist.github.com/Shtille/1f98c649abeeb7a18c5a56696546d3cf
vec3 normalAtPointOnAABB (AABox aabox, vec3 point) {
  float eps = -0.001;
  vec3 hs = (aabox.max - aabox.min) * 0.5;
  vec3 cb = (aabox.min + aabox.max) * 0.5;
  vec3 vp = point - cb;
  return normalize( sign(vp) * step(eps, abs(vp) - hs) );
}

// from: https://jcgt.org/published/0007/03/04/paper.pdf
Intersection intersectAABB (vec3 rayOrg, vec3 rayDir, AABox aabox, Intersection info) {
  vec3 invDir = 1.0 / rayDir;
  vec3 rMin = (aabox.min - rayOrg) * invDir;
  vec3 rMax = (aabox.max - rayOrg) * invDir;
  vec3 vMin = min(rMin,rMax);
  vec3 vMax = max(rMin,rMax);
  float tMin = max(vMin.x,max(vMin.y,vMin.z));
  float tMax = min(vMax.x,min(vMax.y,vMax.z));
  if (tMax < 0.0) return info; // box is behind
  if (tMin > tMax) return info; // never intersects
  //float t = (tMin < 0.0) ? tMax : tMin; // tMin<0=inside
  float t = tMin;
  if ((t > 0.0) && (t < info.t)) {
    info.t = t;
    info.point = rayOrg + rayDir * t;
    info.normal = normalAtPointOnAABB(aabox, info.point);
    info.color = aabox.color;
  }
  return info;
}

Intersection intersectSphere (vec3 rayOrg, vec3 rayDir, Sphere sphere, Intersection info) {
  vec3 v = sphere.origin - rayOrg;
  float b = dot(v,rayDir);
  float d = b * b - dot(v,v) + sphere.radius * sphere.radius;
  if (d < 0.0) return info;
  float root = sqrt(d);
  float t = b - root; // tmin
  //if (t < 0.0) t = b + root; // tmax
  if ((t > 0.0) && (t < info.t)) {
    info.t = t;
    info.point = rayOrg + rayDir * t;
    info.normal = normalize(info.point - sphere.origin);
    info.color = sphere.color;
  }
  return info;
}

Intersection intersectPlane (vec3 rayOrg, vec3 rayDir, Plane plane, Intersection info) {
  float c = dot(rayDir, plane.normal);
  if (c >= 0.0) return info;
  float d = dot(plane.origin, plane.normal);
  float t = -(dot(rayOrg, plane.normal) - d) / c;
  if ((t > 0.0) && (t < info.t)) {
    info.t = t;
    info.point = rayOrg + rayDir * t;
    info.normal = plane.normal;
    // 1x1 checker pattern
    float m = abs(floor(info.point.x) + floor(info.point.z));
    info.color = plane.color * (mod(m,2.0) < 1.0 ? 0.5 : 1.0);
  }
  return info;
}

Intersection intersectScene (vec3 rayOrg, vec3 rayDir)
{
  Intersection info;
  info.t = MAX_DISTANCE;
  //info.point = vec3(...);
  //info.normal = vec3(...);
  info.color = vec3(0.0, 0.0, 0.0); // black background
  
  Plane plane = Plane(
                      vec3(0.0,0.0,0.0), // origin
                      vec3(0.0,1.0,0.0), // normal
                      vec3(1.0,1.0,1.0)); // color
  
  Sphere sphere = Sphere(
                         0.5, // radius
                         vec3(0.0,0.5,0.0), // origin
                         vec3(1.0,1.0,1.0)); // color
  
  AABox aabox = AABox(
                      vec3(-1.5,0.0,0.5), // min
                      vec3(-0.5,2.0,1.5), // max
                      vec3(1.0,1.0,1.0)); // color
  
  
  
  
  return info;
}


// matrix operations adapted from:
// https://www.shadertoy.com/view/ltyXWh
mat4 translate (vec3 t) {
  return mat4(
              vec4(1.0,0.0,0.0,0.0),
              vec4(0.0,1.0,0.0,0.0),
              vec4(0.0,0.0,1.0,0.0),
              vec4(t.x,t.y,t.z,1.0));
}

mat4 scale (vec3 s) {
  return mat4(
              vec4(s.x,0.0,0.0,0.0),
              vec4(0.0,s.y,0.0,0.0),
              vec4(0.0,0.0,s.z,0.0),
              vec4(0.0,0.0,0.0,1.0));
}

mat4 ortho (float l, float r, float b, float t, float n, float f) {
  return scale(vec3(2.0/(r-l),2.0/(t-b),2.0/(f-n))) *
  translate(vec3(-(l+r)/2.0,-(t+b)/2.0,-(f+n)/2.0));
}

mat4 projection (float n, float f) {
  return mat4(
              vec4(n,0.0,0.0,0.0),
              vec4(0.0,n,0.0,0.0),
              vec4(0.0,0.0,n+f,1.0),
              vec4(0.0,0.0,-f*n,0.0));
}

mat4 perspective (float fov, float aspect, float n, float f) {
  float l = tan(fov * 0.5) * n;
  float b = l / aspect;
  return ortho(-l,l,-b,b,n,f) * projection(n,f) * scale(vec3(1.0,1.0,-1.0));
}

mat4 lookAt (vec3 eye, vec3 center, vec3 up) {
  vec3 z = normalize(eye-center);
  vec3 x = normalize(cross(up,z));
  vec3 y = normalize(cross(z,x));
  mat4 m = mat4(
                vec4(x.x,y.x,z.x,0.0),
                vec4(x.y,y.y,z.y,0.0),
                vec4(x.z,y.z,z.z,0.0),
                vec4(0.0,0.0,0.0,1.0));
  return m * translate(-eye);
}
#define PI 3.141592
#define TWOPI 6.283184

#define R2D 180.0/PI*
#define D2R PI/180.0*

mat2 rotMat(in float r){float c = cos(r);float s = sin(r);return mat2(c,-s,s,c);}

//fract -> -0.5 -> ABS  : coordinate absolute Looping
float abs1d(in float x){return abs(fract(x)-0.5);}
vec2 abs2d(in vec2 v){return abs(fract(v)-0.5);}

#define OC 15.0



#define OC 15.0
vec3 Oilnoise(in vec2 pos, in vec3 RGB)
{
  vec2 q = vec2(0.0);
  float result = 0.0;
  
  float s = 2.2;
  float gain = 0.44;
  vec2 aPos = abs2d(pos)*0.5;//add pos
  
  for(float i = 0.0; i < OC; i++)
  {
    pos *= rotMat(D2R 30.);
    float time = (sin(time)*0.015+0.5)*0.2+time*0.08;
    q =  pos * s + time;
    q =  pos * s + aPos + time;
    q = vec2(cos(q));
    
    result += abs1d(dot(q, vec2(0.3))) * gain;
    
    s *= 1.07;
    aPos += cos(q);
    aPos*= rotMat(D2R 5.0);
    aPos*= 1.2;
  }
  
  result = pow(result,4.0);
  return clamp( RGB / result, vec3(0.0), vec3(1.0));
}

void main(  ) {
  vec2 position = (gl_FragCoord.xy/resolution.xy) -0.5;
  
  vec2 uv=(gl_FragCoord.xy*2.-resolution.xy)/min(resolution.x,resolution.y);
  vec3 finalColor=vec3(0);
  
  float time = time * .12;
  float lightPower = 1.0;
  float lightSpeed = 1.0 * time;
  float lightAngle = mod(lightSpeed, 2.0 * 3.14);
  vec3 lightOrg = vec3(
                       8.0 * sin(lightAngle),
                       2.0,
                       8.0 * cos(lightAngle)
                       );
  
  float aspect = dimensions.x / dimensions.y;
  mat4 projMatrix = perspective(radians(90.0),aspect,0.001,1000.0);
  
  vec3 col2 = vec3(0.0,0.0,0.0);
  vec2 st = (coord/dimensions.xy);
  st.x = ((st.x - 0.5) *(dimensions.x / dimensions.y)) + 0.5;
  float stMask = step(0.0, st.x * (1.0-st.x));
  
  
  //st-=.5; //st move centor. Oil noise sampling base to 0.0 coordinate
  st*=3.;
  
  vec3 rgb = vec3(1.30, .8, 1.200);
  
  
  //berelium, 2024-06-07 - anti-aliasing
  float AA = 1.0;
  vec2 pix = 1.0 / dimensions.xy;
  vec2 aaST = vec2(0.0);
  
  for(float i = 0.0; i < AA; i++)
  {
    for(float j = 0.0; j < AA; j++)
    {
      aaST = st + pix * vec2( (i+0.5)/AA, (j+0.5)/AA );
      col2 += Oilnoise(aaST, rgb);
    }
    
  }
  
  col2 /= AA * AA;
  
  vec3 rayOrg = vec3(2.0,2.0,15.0);
  vec3 target = vec3(0.0,0.0,0.0);
  mat4 viewMatrix = lookAt(rayOrg,target,vec3(0.0,1.0,0.0));
  
  vec2 ndcPixel = coord.xy / dimensions.xy * 2.0 - 1.0; // [-1..1]
  vec4 viewDir = inverse(projMatrix) * vec4(ndcPixel, -1.0, 1.0); // view space
  viewDir.w = 0.0; // project to infinity
  vec3 rayDir = normalize((inverse(viewMatrix) * viewDir).xyz); // world space
  
  Intersection info = intersectScene(rayOrg,rayDir);
  
  if (info.t < MAX_DISTANCE)
  {
    //
    // 3D lighting effects
    //
    
    vec3 shadowOrg = info.point;
    vec3 shadowDir = lightOrg - shadowOrg;
    float shadowLen2 = dot(shadowDir,shadowDir);
    float shadowLen = sqrt(shadowLen2);
    shadowDir /= shadowLen; // normalize
    
    Intersection shadowInfo = intersectScene(shadowOrg,shadowDir);
    
    float direct = (shadowInfo.t > shadowLen) ? 1.0 : 0.0;
    float lambertian = max(0.0, dot(info.normal, shadowDir));
    float attenuation = lightPower / shadowLen2;
    float shaders = min(1.0, direct * lambertian * attenuation);
    
    vec3 shadedColor = info.color * shaders;
    vec3 tonedColor = pow(shadedColor, vec3(1.0 / 2.2));
    
    info.color = tonedColor;
  }
  
  
  vec4 sunOrg = vec4(lightOrg, 1.0);
  
  sunOrg = viewMatrix * sunOrg;
  
  sunOrg = projMatrix * sunOrg;
  
  sunOrg.xyz /= sunOrg.w;
  
  sunOrg.xy = (sunOrg.xy + 1.0) * 0.5;
  
  sunOrg.xy = sunOrg.xy * dimensions.xy - 1.0;
  
  
  vec2 cPos = -1.0 + 2.0 * coord.xy / dimensions.xy;
  
  // distance of current pixel from center
  float cLength = length(cPos);
  
  uv+= (cPos/cLength)*cos(cLength*12.0-time*10.0) * u_warp; // 0.03: warp -> u_warp
  
  
  float sunSize = dimensions.y * u_sunSize; // 3.25: sunSize -> u_sunSize
  sunSize /= distance(lightOrg, rayOrg); // scale by view distance
  float sunDist = distance(sunOrg.xy, coord.xy);
  vec3 sunColor = u_sunColor * sunSize / sunDist; // vec3(1.0, 0.5, 2.1): sunColorR, sunColorG, sunColorB -> u_sunColor
  vec3 tonedSunColor = pow(sunColor, vec3(1.0 * u_sunSpread)); // 2.2: sunSpread -> u_sunSpread
  
  // combine 3D and 2D color effects
  info.color = min((info.color + tonedSunColor) * 0.5, vec3(1.0));
  
  
  vec2 p3 = mod(uv * PI2, PI2) - 254.0  ;
  vec2 i = vec2(p3);
  float c3 = 1.2;
  float inten =  0.0064;
  
  c3 /= float(MAX_ITER);
  c3 = 0.22-pow(c3, 1.22);
  vec3 colour = vec3(0.098, 0.098, 0.098+pow(abs(c3), 5.2));
  float c=smoothstep(0.1,0.0,clamp(uv.x*.11+.99,0.,.99));
  
  for (float i = 0.0; i < u_starCount; i++) { // 3.0 starCount -> u_starCount
    c += snow(uv * u_starScale, 3.0 + i * 2.0) * (u_starBrightness - i * 0.1); // 1.0: starScale -> u_starScale, 0.7: starBrightness -> u_starBrightness
  }
  
  vec2 center = dimensions.xy * 0.5;
  
  vec2 orbit = dimensions.yy * 0.4;
  
  float speed = time * 0.1;
  
  float twoPI = 2.0 * 3.14;
  
  float angle = mod(speed * twoPI, twoPI);
  
  vec2 xform = vec2(sin(angle), cos(angle));
  
  vec2 origin = orbit * xform + center;
  
  float size = dimensions.y * 0.1;
  
  float dist = length(coord - origin);
  
  vec3 color = vec3(1.0, 1.0, 1.0) * size / dist;
  vec2 p = gl_FragCoord.xy / resolution.xy;
  vec2 q = p - vec2(0.5, 0.5);
  
  // Time varying pixel color
  vec3 col = 0.5 + 0.5 * sin(vec3(0,2,4) + time / 2.);
  
  float r = 0.4 + 0.1 * cos(atan(-p.x + 2., q.y / 2.) * 60.0 + 20.0 * -q.x + ((length(mouse / .4) / .25) * 2.));
  
  float r1 = 0.2 + 0.1 * cos(atan(-p.x + 2., -p.y / 2.) * 5.0 + 20.0 * -q.x + time * 0.5);
  float r2 = 0.4 + 0.1 * cos(atan(-p.x + 4., -q.y / 2.) * 10.0 + 20.0 * -q.x + time * 0.5);
  float r3 = 0.6 + 0.1 * cos(atan(-p.x + 6., -q.y / 2.) * 20.0 + 20.0 * -q.x + time * 0.5);
  
  col /= smoothstep(r / 100., r, length(q)) / .2;
  
  col.x /= smoothstep(r1 / 100., r1, length(q)) / .2;
  col.y /= smoothstep(r2 / 100., r2, length(q)) / .2;
  col.z /= smoothstep(r3 / 100., r3, length(q)) / .2;
  float y = 0.2 * position.y * sin(200.0*position.y - 20.0*time*1.35);
  outputColor = vec4(y *5., y, y* 5., 1.0);
  y = 1.0 / (600. * abs(position.y - y));
  y += 1./(665.*length(position - vec2(0., position.y)));
  float saule = 1./(65.*length(position - vec2(0, 0)));
  outputColor = vec4(vec3(c*10.0,c*5.1,c*01.9)+ info.color+color*0.1, 1.0);
  
  
  outputColor+=vec4(finalColor*10.+colour*0.25*col2*6.5,1.);
}
