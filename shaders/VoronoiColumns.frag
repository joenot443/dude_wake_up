#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;


// Created by genis sole - 2016
// License Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International.

const float PI = 3.1416;

vec2 hash2( vec2 p )
{
    // procedural white noise
  return fract(sin(vec2(dot(p,vec2(127.1,311.7)),
                          dot(p,vec2(269.5,183.3))))*43758.5453);
}

//Derived from https://iquilezles.org/articles/voronoilines
vec3 voronoi( in vec2 x )
{
    vec2 n = floor(x);
    vec2 f = fract(x);

    //----------------------------------
    // first pass: regular voronoi
    //----------------------------------
  vec2 mg, mr;

    float md = 8.0;
    for( int j=-1; j<=1; j++ )
    for( int i=-1; i<=1; i++ )
    {
        vec2 g = vec2(float(i),float(j));
    vec2 o = hash2( n + g );
    #ifdef ANIMATE
        o = 0.5 + 0.5*sin( time + 6.2831*o );
        #endif
        vec2 r = g + o - f;
        float d = dot(r,r);

        if( d<md )
        {
            md = d;
            mr = r;
            mg = g;
        }
    }

    //----------------------------------
    // second pass: distance to borders
    //----------------------------------
    md = 8.0;
    vec2 ml = vec2(0.0);
    for( int j=-2; j<=2; j++ )
    for( int i=-2; i<=2; i++ )
    {
        vec2 g = mg + vec2(float(i),float(j));
    vec2 o = hash2( n + g );
    #ifdef ANIMATE
        o = 0.5 + 0.5*sin( time + 6.2831*o );
        #endif
        vec2 r = g + o - f;

        if( dot(r-mr,r-mr)>0.00001 ) {
            vec2 l = normalize(r-mr);
            float d = dot( 0.5*(mr+r), l );
            if (md > d) {
            md = d;
                ml = l;
            }
        }
    }

    return vec3(md, ml);
}



void voronoi_s(in vec2 x, inout vec2 n,  inout vec2 f,
                          inout vec2 mg, inout vec2 mr) {

    n = floor(x);
    f = fract(x);

    float md = 8.0;
    for( int j=-1; j<=1; j++ )
    for( int i=-1; i<=1; i++ )
    {
        vec2 g = vec2(float(i),float(j));
    vec2 o = hash2( n + g );
        vec2 r = g + o - f;
        float d = dot(r,r);

        if (d < md) {
            md = d;
            mr = r;
            mg = g;
        }
    }
}

vec3 voronoi_n(inout vec2 rd, inout vec2 n,  inout vec2 f,
                              inout vec2 mg, inout vec2 mr) {
    float md = 1e5;
    vec2 mmg = mg;
    vec2 mmr = mr;
    vec2 ml = vec2(0.0);
    
    for( int j=-2; j<=2; j++ )
    for( int i=-2; i<=2; i++ )
    {
        vec2 g = mmg + vec2(float(i),float(j));
    vec2 o = hash2( n + g );
    vec2 r = g + o - f;

      vec2 l = r-mmr;
     if((dot(l, l) * dot(rd, l)) > 1e-5) {
            float d = dot(0.5*(mmr+r), l)/dot(rd, l);
            if (d < md) {
                md = d;
                mg = g;
                mr = r;
                ml = l;
            }
        }
    }
    
    return vec3(md, ml);
}


bool IRayAABox(in vec3 ro, in vec3 rd, in vec3 invrd, in vec3 bmin, in vec3 bmax,
               out vec3 p0, out vec3 p1)
{
    vec3 t0 = (bmin - ro) * invrd;
    vec3 t1 = (bmax - ro) * invrd;

    vec3 tmin = min(t0, t1);
    vec3 tmax = max(t0, t1);
    
    float fmin = max(max(tmin.x, tmin.y), tmin.z);
    float fmax = min(min(tmax.x, tmax.y), tmax.z);
    
    p0 = ro + rd*fmin;
    p1 = ro + rd*fmax;
 
    return fmax >= fmin;
}

vec3 AABoxNormal(vec3 bmin, vec3 bmax, vec3 p)
{
    vec3 n1 = -(1.0 - smoothstep(0.0, 0.03, p - bmin));
    vec3 n2 = (1.0 -  smoothstep(0.0, 0.03, bmax - p));
    
    return normalize(n1 + n2);
}

const vec3 background = vec3(0.04);
const vec3 scmin = -vec3(1.77, 1.0, 1.77);
const vec3 scmax = vec3(1.77, 1.5, 1.77);


float map(in vec2 p) {
    vec2 h = exp(-(p*p*0.5));
    return scmin.y + 0.1 + hash2(p).x * 0.5
//        + texture(tex, vec2(hash2(p).x, 0.0)).r*2.0 + h.x*h.y*0.5;
          + 1.0 + h.x*h.y*0.5;
}

// From https://iquilezles.org/articles/palettes
vec3 pal( in float t, in vec3 a, in vec3 b, in vec3 c, in vec3 d )
{
    return a + b*cos( 6.28318*(c*t+d) );
}

vec3 color(vec2 p) {
    return 1.5 *
           pal(3.434+(hash2(p).x*0.02), vec3(0.5,0.5,0.5),
                                        vec3(0.5,0.5,0.5),
                                        vec3(1.0,0.7,0.4),
                                        vec3(0.0,0.15,0.20)  );
}

float voronoi_ao(in float h, inout vec2 n, inout vec2 f, inout vec2 mg) {
    float a = 0.0;
    
    for (int j = -1; j <= 1; ++j)
    for (int i = -1; i <= 1; ++i)
    {
      vec2 g = mg + vec2(float(i), float(j));
        a += max(0.0, map(n + g) - h);
    }
    
    return exp(-a*0.5) + 0.2;
    //return max(0.0, 1.0 - a*0.2) + 0.2;
}

vec2 ShadowAndAmbient(in vec3 ro, in vec3 rd) {
    vec3 p0 = vec3(0.0), p1 = vec3(0.0);
    
    IRayAABox(ro, rd, 1.0/rd, scmin, scmax, p0, p1);
    
    if (length(ro - p1) < 0.01) return vec2(1.0);
    
    p0 = ro + rd*0.01;
    vec2 dir = normalize(rd.xz);
    float s = rd.y / length(rd.xz);
    
    vec2 mg = vec2(0.0), mr = vec2(0.0), n = vec2(0.0), f = vec2(0.0);
    voronoi_s(p0.xz*2.0, n, f, mg, mr);
    
    float h = map(n + mg);
    
    float a = voronoi_ao(ro.y, n, f, mg);
    vec3 dn = voronoi_n(dir, n, f, mg, mr);
    
    float rh = 0.0, prh = p0.y;
    
    float dmax = length(p0.xz - p1.xz);
  float mh = 1.0;
    
    const int steps = 8;
    for (int i = steps; i > 0; --i) {
        dn.x *= 0.5;
        rh = p0.y + dn.x*s;
       
        if (dn.x > dmax || h > prh || h > rh) break;
        
        prh = rh;
       
        h = map(n + mg);
        
        mh = min(mh, 14.0*(rh-h)/(dn.x*s));
        
        dn = voronoi_n(dir, n, f, mg, mr);
    }
    
    if (h > prh || h > rh) return vec2(0.0, a);
    
    return vec2(clamp(mh, 0.0, 1.0), a);
}

vec3 Shade(in vec3 p, in vec3 p0, in vec3 v, in vec3 n, in vec3 l, in vec2 c) {
    vec3 col = color(c);
    vec2 lc = vec2(max(0.0, dot(n,l))*0.9, 1.0);
    
    if (length(p - p0) < 0.01) return 3.0*col*(lc.x + lc.y)*0.5;
    
    vec2 sa = ShadowAndAmbient(p + n*0.02, l);
     lc *= sa;
    return col*(lc.x + lc.y);
}

vec3 Render(in vec3 ro, in vec3 rd, in vec3 ld) {
    vec3 p0 = vec3(0.0), p1 = vec3(0.0);
    
    if (!IRayAABox(ro, rd, 1.0/rd, scmin, scmax, p0, p1)) return background;
    
    vec2 dir = normalize(rd.xz);
    float s = rd.y / length(rd.xz);
    
    vec2 mg = vec2(0.0), mr = vec2(0.0), n = vec2(0.0), f = vec2(0.0);
    voronoi_s(p0.xz*2.0, n, f, mg, mr);
    
    vec2 pmg = mg, pmr = mr;
    float h = map(n + pmg);
    
    vec3 dn = voronoi_n(dir, n, f, mg, mr);
    vec3 pdn = vec3(0.0, -AABoxNormal(scmin, scmax, p0).xz);
    
    float rh = 0.0, prh = p0.y;
    
    float dmax = length(p0.xz - p1.xz);
  
    const int steps = 16;
    for (int i = steps; i > 0; --i) {
        dn.x *= 0.5;
        rh = p0.y + dn.x*s;
        
        if (dn.x > dmax || h > prh || h > rh) break;
        
        prh = rh;
        
        pmg = mg; pmr = mr;
        h = map(n + pmg);
        
        pdn = dn;
        dn = voronoi_n(dir, n, f, mg, mr);
    }
    
    if (!(h > prh || h > rh)) return background;
    
    vec3 p = vec3(p0.xz + dir*pdn.x, p0.y + pdn.x*s).xzy;
    vec3 nor = vec3(normalize(-pdn.yz), 0.0).xzy;
    
    if (h < prh) {
      float dt = abs((p0.y - h)/s);
        
        if (dt > dmax) return background;
        
        p = vec3(p0.xz + dir*dt, h).xzy;
        
        vec3 v = voronoi(p.xz*2.0)*0.5;
        nor = normalize(mix((vec3(normalize(v.yz), 0.0).xzy),
                            vec3(0.0, 1.0, 0.0), smoothstep(0.0, 0.03, v.x)));
    }
    
    return Shade(p, p0, rd, nor, ld, n + pmg);
    
}

void CameraOrbitRay(in vec2 coord, in float n, in vec3 c, in float d,
                    out vec3 ro, out vec3 rd, out mat3 t)
{
    float a = 1.0/max(dimensions.x, dimensions.y);
    rd = normalize(vec3((coord - dimensions.xy*0.5)*a, n));
 
    ro = vec3(0.0, 0.0, -d);
    
    float ff = min(1.0, step(0.001, 0.5) + step(0.001, 0.5));
    vec2 m = PI*ff + vec2(((vec2(0.6, 0.6)) / dimensions.xy) * (PI*2.0));
    m.y = -m.y;
    m.y = sin(m.y*0.5)*0.6 + 0.6;
        
    mat3 rotX = mat3(1.0, 0.0, 0.0, 0.0, cos(m.y), sin(m.y), 0.0, -sin(m.y), cos(m.y));
    mat3 rotY = mat3(cos(m.x), 0.0, -sin(m.x), 0.0, 1.0, 0.0, sin(m.x), 0.0, cos(m.x));
    
    t = rotY * rotX;
    
    ro = t * ro;
    ro = c + ro;

    rd = t * rd;
    
    rd = normalize(rd);
}

vec3 LightDir(in mat3 t)
{
    vec3 l = normalize(vec3(1.0, 1.0, -1.0));
    return t * l;
}

void main(  )
{
    vec3 ro = vec3(0.0);
    vec3 rd = vec3(0.0);
    mat3 t = mat3(1.0);
    
    CameraOrbitRay(coord, 1.0, vec3(0.0), 10.0, ro, rd, t);
  outputColor = vec4(pow(Render(ro, rd, LightDir(t)), vec3(0.5454)), 1.0);
}
