#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform vec2 position;

uniform float zoom;
uniform int   octaves;
uniform float lacunarity;
uniform float gain;
uniform float focusPull;
uniform float brightness;
uniform float hueShift;

uniform float domainWarp;
uniform float perspective;
uniform vec4  glowColor;
uniform float saturation;
uniform float vignette;

in vec2 coord;
out vec4 outputColor;

//philip.bertani@gmail.com


float focus = 0.;
float focus2 = 0.;
float tf = 0.;

#define pi  3.14159265

float random(vec2 p) {
    //a random modification of the one and only random() func
    return fract( sin( dot( p, vec2(12., 90.)))* 1e6 );
}


float torus(vec3 p, vec2 axis) {

    vec2 q = vec2( length(p.xy) - axis.x, p.y);
    return length(q) - axis.y;
}


//this is taken from Visions of Chaos shader "Sample Noise 2D 4.glsl"
float noise(vec3 p) {
    vec2 i = floor(p.yz);
    vec2 f = fract(p.yz);
    float a = random(i + vec2(0.,0.));
    float b = random(i + vec2(1.,0.));
    float c = random(i + vec2(0.,1.));
    float d = random(i + vec2(1.,1.));
    vec2 u = f*f*(3.-2.*f); //smoothstep here, it also looks good with u=f

    return mix(a,b,u.x) + (c-a)*u.y*(1.-u.x) + (d-b)*u.x*u.y;

}

float fbm3d(vec3 p) {
    float v = 0.;
    float a = .5;
    vec3 shift = vec3(focus - focus2);  //play with this
    shift.z += .03*tf;

    float angle = pi/4.;
    float cc=cos(angle), ss=sin(angle);
    mat3 rot = mat3( cc,  0., ss,
                      0., 1., 0.,
                     -ss, 0., cc );
    for (int i=0; i<octaves; i++) {
        v += a * noise(p);
        p = rot * p * lacunarity + shift;
        a *= gain * (1. + focusPull * (focus + focus2));
    }
    return v;
}

mat3 rxz(float an){
    float cc=cos(an),ss=sin(an);
    return mat3(cc,0.,-ss,
                0.,1.,0.,
                ss,0.,cc);
}
mat3 ryz(float an){
    float cc=cos(an),ss=sin(an);
    return mat3(1.,0.,0.,
                0.,cc,-ss,
                0.,ss,cc);
}

// Hue rotation in YIQ space — compact and branch-free.
vec3 rotateHue(vec3 c, float t) {
    const mat3 toYIQ = mat3(0.299, 0.596, 0.211,
                            0.587,-0.274,-0.523,
                            0.114,-0.322, 0.312);
    const mat3 toRGB = mat3(1.0,   1.0,   1.0,
                            0.956,-0.272,-1.106,
                            0.621,-0.647, 1.703);
    vec3 yiq = toYIQ * c;
    float a = t * 6.28318530718;
    float cs = cos(a), sn = sin(a);
    yiq.yz = mat2(cs, -sn, sn, cs) * yiq.yz;
    return toRGB * yiq;
}

void main(  )
{

    float coord_scale = 2. * zoom;
    float tt = time / 8.;
    vec2 uv = (2.*coord-dimensions.xy)/dimensions.y;
    vec2 mm = (2.*position*dimensions.xy-dimensions.xy)/dimensions.y;
//  vec2 mm = (2.*iMouse.xy-dimensions.xy)/dimensions.y;

//    if ( iMouse.w == 0. ) mm.xy += vec2(1.,.5);

    uv *= coord_scale;

    vec3 rd = normalize( vec3(uv, -perspective) );
    vec3 ro = vec3(0.);

    float delta = pi/100.;
    mat3 rot = rxz(-mm.x*delta) * ryz(-mm.y*delta);

    ro -= rot[2] * time / 5.;

    vec3 p = ro + rot*rd;


    float ttt = 1.55;

    //distance to torus
    float td = torus( p - ro , vec2( 4.*cos(time/2.)*cos(ttt*5.),8.*sin(ttt*5.)) );


    mm *= coord_scale;

    focus = length(uv-mm);
    focus = sqrt(focus);
    focus = 2.5/(1.+focus/2.);

    focus2 = length(uv+mm );
    focus2 = 1./(1.+focus2*focus2);

    tf = .1 * exp(-td);


    vec3 q;
    q.x = fbm3d(p);
    q.y = fbm3d(p.yzx);
    q.z = fbm3d(p.zxy);

    float f = fbm3d(p + q * domainWarp);

    vec3 cc = q;
    cc *= 35. * f * brightness;


    cc.r += 2.*focus*focusPull; cc.g += 2.*focus*focusPull; cc.b += 8.*focus2*focusPull; cc.r -= 4.*focus2*focusPull;
    cc /= 10.;
    cc /= (1.+.006*tf);
    cc += .001*tf*glowColor.rgb*glowColor.a;

    cc = 1. - exp(-cc*cc);
    //cc = pow(cc, vec3(3.));

    cc = rotateHue(cc, hueShift);

    // Saturation around per-pixel luma (post-hue so the rotation isn't
    // pre-flattened).
    float luma = dot(cc, vec3(0.299, 0.587, 0.114));
    cc = mix(vec3(luma), cc, saturation);

    // Vignette: darken corners based on normalized screen distance from
    // center. Independent of the focus-driven brightening above.
    vec2 vCentered = (coord / dimensions.xy) * 2.0 - 1.0;
    float vd = length(vCentered);
    cc *= 1.0 - smoothstep(0.4, 1.2, vd) * vignette;

    outputColor = vec4(cc,1.0);

}

