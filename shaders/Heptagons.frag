#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

/*
    
    Half Cross Heptagon Pattern
    ---------------------------
    
    Covering a plane with crosses, then subdividing them through opposite 
    inner vertices to create a single non-convex heptagon tiling of the 
    Euclidean plane.
    
    It might be more obvious to others, but when I first encountered one 
    of these non-convex heptagon patterns, it took me ages to realize that 
    it was just a reimagining of tiled, rotated crosses that were cut in 
    half. In fact, once you realize this, the pattern loses a bit of its 
    mystique. However, I haven't seen one posted on Shadertoy before, and 
    it still looks interesting to me, so I figured I'd post it anyway. :)
    
    As an aside, I tend to prefer the term "septagon", which I think is a 
    misusage of latin and Greek to describe a seven sided object, but I'm 
    pretty sure that heptagon is more preferred technical term.
    
    There's not much to say about the pattern construction: Tile the plane 
    with Greek style crosses (any dimensions), then subdivide them through 
    opposite inner vertices. The details are below.
    
    By the way, I have a 3D traversal version of this pattern that I'll 
    post pretty soon.
    
    
    
    Other examples:
    
    // If you exclude some of the hyperbolic Poincare posting,
    // heptagon based examples are few and far between on Shadertoy.
    // Here's one that I'd forgotten I'd written.
    Heptagon-Pentagon Tiling - Shane
    https://www.shadertoy.com/view/wtByzh

*/

// Looking at a visual of a repeat grid that produces a pattern can be very 
// helpful. In fact, patterns are often difficult to decipher without one.
//#define GRID


// Pattern type -  Crosses: 0, Heptagons: 1.
#define PATTERN 1

// Standard 2D rotation formula.
mat2 rot2(in float a){ float c = cos(a), s = sin(a); return mat2(c, -s, s, c); }

uint customFloatBitsToUint(float f) {
    // Split the float into components we can work with manually
    vec4 bitShifter = vec4(1.0, 255.0, 65025.0, 16581375.0);
    
    // Convert the float to a normalized 32-bit representation
    vec4 components = fract(f * bitShifter);
    
    // Shift the components and reassemble them into an unsigned integer
    uint result = uint(components.x * 255.0) +
                  uint(components.y * 255.0 * 256.0) +
                  uint(components.z * 255.0 * 65536.0) +
                  uint(components.w * 255.0 * 16777216.0);
    
    return result;
}

// Fabrice's fork of "Integer Hash - III" by IQ: https://shadertoy.com/view/4tXyWN
float hash21(vec2 f){
    
    uvec2 p = uvec2(customFloatBitsToUint(f.x + 512.), customFloatBitsToUint(f.y + 512.));
    p = 1664525U*(p>>1U^p.yx);
    return float(1103515245U*(p.x^(p.y>>3U)))/float(0xffffffffU);
}

// Commutative smooth maximum function. Provided by Tomkh, and taken 
// from Alex Evans's (aka Statix) talk: 
// http://media.lolrus.mediamolecule.com/AlexEvans_SIGGRAPH-2015.pdf
// Credited to Dave Smith @media molecule.
float smax(float a, float b, float k){
    
   float f = max(0., 1. - abs(b - a)/k);
   return max(a, b) + k*.25*f*f;
}

// Signed distance to a line passing through A and B.... Not a proper
// signed line distance function, but it works for many things.
float distLineS(vec2 p, vec2 a, vec2 b){

   b -= a; 
   return dot(p - a, vec2(-b.y, b.x)/length(b));
}


// IQ's cross distance field.
//
// Signed distance to a 2D cross. Produces exact exterior and
// bound interior distance.
//
// List of some other 2D distances: https://www.shadertoy.com/playlist/MXdSRf
//
// and iquilezles.org/articles/distfunctions2d
float sdCross(in vec2 p, in vec2 b, float r){ 

    p = abs(p); p = (p.y>p.x) ? p.yx : p.xy;
    
	vec2  q = p - b;
    float k = max(q.y, q.x);
    vec2  w = (k>0.) ? q : vec2(b.y - p.x, -k);
    float d = length(max(w, 0.));
    return ((k>0.)? d: -d) + r;
}


// Global scale and local coordinates.
vec2 gSc = vec2(1)/4.;
//vec2 gP; // Not used here.

vec4 pattern(vec2 p){
    
    
    // Overall coordinates and scale.
    vec2 sc = gSc;
    
    // The cross has thickness, one third of the height, which is evident from
    // the imagery. If you turn on the grid settings, you'll see that a one to
    // three ratio triangle is involved, etc.
    // Cross rotation angle.
    float a = atan(1., 3.);
    // Cross height: This follows from the above. Feel free to get out a pen and
    // paper, or you could take a lazy coder's word for it. :D
     float ht = sqrt(9./10.);//cos(a);//
    //a = acos(ht);
    // The width needs to be one third of the height in order for two equal size
    // crosses to tile the plane.
    vec2 si = sc*vec2(ht, ht/3.)/2.;
    
    // Cross ID. Only two crosses are needed to tile the plane.
    int crossID = 0;
    // Each cross can be subdivided into two septagons.
    int polyID = 0;
    
    
    // Square grid setup. Cell ID and local coordinates.
    vec2 q = p;
    vec2 iq = floor(q/sc) + .5;
    q -= (iq)*sc;
    
    // Matrix to rotate the crosses.
    mat2 m = rot2(a);
    
    float rF = .072*sc.x; // Polygon rounding factor.
     
    // The first cross.
    //
    // Render a slightly rounded cross using IQ's cross distance field function.
    // As an aside, if cell traversing, you'd test again a cross bound, rather
    // than a distance field version.
    float d2 = sdCross(m*q, si - rF, -rF);
    float d = d2; // Overal distance set to the first cross.
    vec2 id = iq; // First cross ID.
    vec2 lP = q; // The cross is rotated, but the local space is not.
    
    // Next cross.
    q = p - sc/2.; //vID[2]*sc
    iq = floor(q/sc) + .5;
    q -= (iq)*sc;
 
    // The second cross.
    d2 = sdCross(m*q, si - rF, -rF);
    
    // See if this cross is closer, and update, if necessary.  
    if(d2<d){
       d = d2;
       id = iq + .5;
       lP = q;
       crossID = 1;
    }
    
    // Individual cross position based ID.
    vec2 cID = id;
    
  
    #if PATTERN != 0

    // Original cell vertex and mid-edge IDs. I could streamline this, but
    // it's working, so I'll leave it alone.
    const mat4x2 vID = mat4x2(vec2(-.5), vec2(-.5, .5), vec2(.5), vec2(.5, -.5));
    const mat4x2 eID = mat4x2(vec2(-.5, 0), vec2(0, .5), vec2(.5, 0), vec2(0, -.5));
   
    // Starting cross dividing line vertex index.
    int n0 = hash21(id)>.5? 0 : 1;
    //int n0 = mod(floor(floor(cID.x)) + floor(floor(cID.y)*2.), 2.)==0.? 0 : 1;
 
    // Half cross dividing line. There are a few ways to divide a 2D distance
    // field, but this is my "go to".
    float divLn = distLineS(m*lP, vID[(n0 + 2)%4], vID[n0%4]);
    int n;
    if(divLn<0.){
        // Heptagon 1.
        d = smax(d, divLn, .007); // Smooth maximum, for a bit of rounding.
        polyID = 0;
        n = n0;
    }
    else{
        // Heptagon 2.
        d = smax(d, -divLn, .007);
        polyID = 1;
        n = n0 + 2;
    }

    
    // Update the position based ID.
    id += vID[(n + 1)%4]/6.;
    
    
    // Strategic vertex and edge based holes. Originally for debugging,
    // but they have an interesting aesthetic, so I kept some around. :)
    
    float hSz = .02*sc.x; // Hole size.
    //d = max(d, -(length(m*lP - vID[(n + 1)%4]*sc/6.) - hSz));

    if(hash21(id + .23)<.35){
        d = max(d, -(length(m*lP - eID[n]*sc/1.732) - hSz));
        d = max(d, -(length(m*lP - eID[(n + 1)%4]*sc/1.732) - hSz));
    } 
   
    #endif
    
    // Not actually used for this example, but needed for others.
    //gP = m*lP; // Rotating the local coordinates to match the rotated crosses.
    
    // Distance, cross and pentagon ID, and cell ID.
    return vec4(d, crossID*2 + polyID, id);
}


float gridField(vec2 p){
    
    vec2 sc = gSc;
    vec2 ip = floor(p/sc);
    p -= (ip + .5)*sc;
    
    p = abs(p) - .5*sc;
    float grid = abs(max(p.x, p.y)) - .007*sc.x;
    
    return grid;
}


// A very simple random line routine. It was made up on the
// spot, so there would certainly be better ways to do it.
float randLines(vec2 p){
    
    // Scaling.
    float sc = 16./gSc.x;
    p *= sc;
    
    // Offset the rows for a more random look.
    p.x += hash21(vec2(floor(p.y), 7) + .2)*sc;
    
    // Cell ID and local coordinates.
    vec2 ip = floor(p);
    p -= ip + .5;
    
    // Distance field value and random cell number.
    float d;
    float rnd = hash21(ip + .34);
    
    // Randomly, but not allowing for single dots.
    if(rnd<.333 && mod(ip.x, 2.)==0.){
    
       // Dots on either side of the cell wall mid-points, to create a space.
       d = min(length(p - vec2(-.5, 0)), length(p - vec2(.5, 0)));
        
    }
    else {
        // Otherwise, just render a line that extends beyond the cell wall
        // mid-points.
        d =  abs(distLineS(p, vec2(-1, 0), vec2(1, 0)));
    }
    
    // Applying some width.
    d -= 1./6.;
    
    // Scaling down the distance value to match scaling up
    // the coordinates.
    return d/sc;
}

void main(){

    // Aspect correct screen coordinates.
	vec2 uv = (coord - dimensions.xy*.5)/dimensions.y;
    
    // Fake distortion. The UV coordinates have been reexpanded along X
    // to fill up the screen length.
    vec2 uv2 = uv*vec2(dimensions.y/dimensions.x, 1.);
    uv *= 1. + (dot(uv2, uv2) - .5)*.2;
    
    // Scaling, smoothing factor and translation.
    float gSc = 1.;
    float sf = gSc/dimensions.y*gSc;
    
    //rot2(3.14159/4.)*
    // Depending on perspective; Moving the oject toward the bottom left, 
    // or the camera in the north east (top right) direction. 
    vec2 p = rot2(3.14159/9.)*rot2(-atan(1., 3.))*uv*gSc - vec2(-1, -.5)*time/12.;//
    
    // 2D directional light vector and a nearby sample, in order 
    // to add some directional derviative based highlighting.
    vec2 ld = normalize(vec2(-1.5, -1));
    vec4 d4Hi = pattern(p - ld*.005);
    float poly2 = d4Hi.x;

    vec4 d4 = pattern(p);
    float poly = d4.x;
    int oID = int(d4.y);
    vec2 id = d4.zw;
  
    // Transcendental based coloring. I don't know if IQ invented it, but
    // his examples are the reason everyone uses it now. :)
    float rnd = hash21(d4.zw + .11);
    vec3 oCol = .5 + .45*cos(6.2831*rnd/4. + vec3(0, 2, 1));
   
    /*
    // Trying to introduce more colors... It didn't work. :)
    
    int crossID = oID/2; // One of two cross IDs. 
    int polyID = oID%2; // One of two polygon IDs.
    //if((crossID + polyID)%2==0)  oCol = vec3(.35)*dot(oCol, vec3(.299, .587, .114));; 
    if((int(d4.z/2.) + int(d4.w/2.))%2==0){
         oCol = mix(oCol, oCol.yxz, .9);
    } 
    // Greyscale. 
    //if(hash21(d4.zw + .13)<.5) oCol = oCol.zzz;
    */
     
    
    // Directional derivative based bump mapping... Not as good as real gradient
    // based bump mapping (usually one extra sample), but it has a certain naive 
    // appeal to it.
    float bump = max(min(-poly/gSc, .016) - min(-poly2/gSc, .016), 0.)/.005;
    bump += max(min(-poly/gSc, .036) - min(-poly2/gSc, .036), 0.)/.005/3.;
    //bump += max(-poly/gSc - -poly2/gSc, 0.)/.005*.25;
    //
    // Applying the bump highlighting.
    oCol *= (bump + bump*bump + 1.);
    
    // Initializing to the background color.
    vec3 col = vec3(.1);
    float ew = .007; // Dark edge width.
    
    // Rendering the objects themselves to the background.
    col = mix(col, vec3(0), 1. - smoothstep(0., sf, poly));
    col = mix(col, oCol, 1. - smoothstep(0., sf, poly + ew));
    
    // Saving the color here for bump highlighting, if the same
    // bump color is desired. You'll see that the bump colors are
    // different, which isn't really physically correct, but it
    // looks interesting.
    //vec3 bCol = col;
    //if(hash21(d4.zw + .41)<.5) bCol = bCol.yxz;
   
    
    // Randomly color half of the tiles with something complimentary.  
    if(hash21(d4.zw + .41)<.5) col = col.yzx;
    // Greyscale.
    //if(hash21(d4.zw + .22)<.35) col = vec3(col.z*.8);
    
    // Adding a random line pattern to the background.
    float pat = randLines(rot2(-3.14159/4.)*p);
    col = mix(col*1.1, col*.9, 1. - smoothstep(0., sf, pat));

     
    // Applying the highlighting, or bump mapping, if you prefer.
    col = mix(pow(col, vec3(1))*1., col.xzy, min(bump*1., 1.));
    
    // Applying some light attenuation.
    uv2 = uv - vec2(.75, .5);
    uv2 *= rot2(sin(time/4.)*.25  + .5);
    float atten = 1. - 1./(1. + pow(max(uv2.y*.25 + .5, 0.), 2.)*16.);
    /*
    // Spotlight: It's an interesting effect, but doesn't really work here.
    float light = abs(uv2.x) - .3*(-uv2.y*1.);
    col = mix(col*.65, col*vec3(1, .97, .92)*1.5, 
              (1. - smoothstep(0., .05, light))*atten);
    */
    col *= atten*.8 + .4;

    // Grid.
    #ifdef GRID
    float grid = gridField(p);
    col = mix(col, col*0., 1. - smoothstep(0., sf, grid));
    #endif
    
    // Vignette.
    uv = coord/dimensions.xy;
    col *= pow(16.*uv.x*uv.y*(1. - uv.x)*(1. - uv.y), 1./16.)*1.05;

    
    // Output to screen
    outputColor = vec4(sqrt(max(col, 0.)), 1);
}
