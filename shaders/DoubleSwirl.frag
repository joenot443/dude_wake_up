#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

// === Drawing transformed-space isolines using screen-space metrics ================
// Application (and cleaning) of https://shadertoy.com/view/Xlffzj
// Conformal version in https://www.shadertoy.com/view/MllBzj

#define S(v) smoothstep(2./dimensions.y, 0., v )

// --- direct transforms
vec2 Mobius(vec2 p, vec2 z1, vec2 z2)
{
	z1 = p - z1; p -= z2;
	return mat2(z1,z1.y,-z1.x) * p / dot(p, p);
}

vec2 spiralZoom(vec2 p, vec2 offs, float n, float spiral, float zoom, vec2 phase)
{
	p -= offs;
	float a = atan(p.y, p.x)/6.283 + time/32.;
	float d = length(p);
	return mat2(n,1, spiral,-zoom) * vec2(a, log(d)) + phase;
}

// --- inverse transforms
vec2 iMobius(vec2 p, vec2 z1, vec2 z2)     
{                               
    float s = sign(p.x), sb = sign(p.y),             // make unique sol: s=1,x1,y1 otherwise 4
          l = length(p), t = p.y/p.x,                // solve  u = v*l ; tan(u,v) = p.y/p.x
          c = s / sqrt(1.+t*t),                      // c = cos(atan( ) )
        v = length(z1-z2) / sqrt( 1.+ l*l -2.*l*c ), // c = (u²+v²-|z1z2|² ) / 2uv
        u = v*l;                                   
    vec2  a = 2.*(z1-z2);                            // solve |P-z1| = u ; |P-z2| = v
    float b = u*u-v*v + dot(z2,z2)-dot(z1,z1);       // ||²-||²: a.P + b = 0
    // y = -( b + a.x* x ) / a.y                     // normalize by a.y
    b /= a.y; a /= a.y;                              // ( in Shane example a.y was = 0 ! )
    float A = 1. +  a.x*a.x,                         //  |P-z1|² = u² , P = (x,y)
          B =     2.* b*a.x -2.*dot(vec2(1,-a.x),z1),
          C =           b*b +2.*b*z1.y + dot(z1,z1) - u*u,
          D = max(0., B*B - 4.*A*C),
         x1 = (-B+sb*sqrt(D))/(2.*A), y1 = -( b + a.x*x1 );
     //  x2 = (-B-sb*sqrt(D))/(2.*A), y2 = -( b + a.x*x2 );
  // if (abs(A)<1e-5) { x1 = x2 = -C/B; y1 = y2 = -( b + a.x*x1 ) / a.y; } // degenerate case
	return vec2(x1,y1);                    
}

vec2 ispiralZoom(vec2 p, vec2 offs, float n, float spiral, float zoom, vec2 phase)
{
    p = inverse(mat2(n,1, spiral,-zoom)) * (p-phase) ;
    p.x = (p.x - time/32.) * 6.283;             // ( p.x, p.y )  = ( a, log(d) )
    return exp(p.y) * vec2(cos(p.x),sin(p.x)) + offs;
}

// --- demo configuration 
vec2 Z1 = vec2(-.875, -.525),  // in Shane example Z1.y=Z2.y was causing a degenerescence
     Z2 = vec2(.375, -.125);

vec2 MobiusSpiral(vec2 q)      // total direct transform
{
    q = Mobius(q, Z1, Z2);
    return spiralZoom(q, vec2(-.5), 5., 3.14159*.2, .5, vec2(-1, 1)*time*.25);
}
float DrawInvMobiusSpiral(vec2 q, vec2 uv0, float r) // total inverse transform + draw iso-X
{
    vec2 v; float s = 0.;
    q = ispiralZoom(q, vec2(-.5), 5., 3.14159*.2, .5, vec2(-1, 1)*time*.25);
    v = iMobius(q, Z1, Z2);  
    return S(length(uv0-v) - r ); // * (.5+.5* cos( 3.14 * length(uv0-v) / r / 2.) );
}

#define rnd(p) fract( 43758.5453 * sin( dot(p, vec2(12.9898, 78.233))) )

void main()
{
  vec2 U = coord;
	vec2 R = dimensions.xy,
        uv0 = (U -.5*R) / R.y,
    uv = MobiusSpiral(uv0);
#define f(v) fract(v+.5)-.5                                    // suppress the wrapping glitch
    mat2 M = inverse(mat2(f(dFdx(uv)),f(dFdy(uv)))) / max(360.,R.y); // measure compression, for LOD
     
    outputColor = vec4(0.0);
    float l,a, r=.005; vec2 q;
    for (int i=0; i<2; i++) {
        //q = floor(uv*4.)/4.;
        
        l = log2(20.*length(M[1]));
        a = fract(l);
        l = exp2(floor(l));                                    // compute LOD
      //l = exp2(floor(l+rnd(uv0)));                           // variant: fading by dithering
        q = vec2( floor(uv.x*l+float(i)) / l, uv.y );          // H isolines in screen space
        outputColor += (1.-a)* DrawInvMobiusSpiral(q, uv0, r );
        q = vec2( floor(uv.x*(l*2.)+float(i)) / (l*2.), uv.y );// inter-LOD fading
        outputColor += a* DrawInvMobiusSpiral(q, uv0, r );
#if 0    
        l = exp2(floor(log2(100.*length(M[0])))); 
        q = vec2( uv.x, floor(uv.y*l+float(i)) / l );          // V isolines in screen space
        outputColor.b += DrawInvMobiusSpiral(q, uv0, r );
#endif
#if 0            
        for (int j=0; j<2; j++) {
            q = floor(uv*4. +vec2(i,j) ) / 4.;                 // dots in screen space
            outputColor.g += DrawInvMobiusSpiral(q, uv0, .02 );
        }
#endif        

    }    
} 