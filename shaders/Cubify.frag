#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float cubeSize;
in vec2 coord;
out vec4 outputColor;

uvec3 pcg3d(uvec3 v) {
    v = v * 1664525u + 1013904223u;
    v.x += v.y*v.z; v.y += v.z*v.x; v.z += v.x*v.y;
    v = v ^ (v >> 16u);
    v.x += v.y*v.z; v.y += v.z*v.x; v.z += v.x*v.y;
    return v;
}

vec3 pcg3d(vec3 v) {
    uvec3 uv = uvec3(abs(v + 10000.0));
    uvec3 r = pcg3d(uv);
    return vec3(r & uvec3(0xffff)) * 1.0/65535.0;
}

vec2 intersectUnitBox(vec3 rayOrigin, vec3 rayDir) {
    vec3 tMin = (vec3(-1.0) - rayOrigin) / rayDir;
    vec3 tMax = (vec3(1.0) - rayOrigin) / rayDir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return vec2(tNear, tFar);
}

vec3 normalUnitBox(vec3 p) {
    vec3 ap = abs(p);
    bvec3 sel = greaterThan(ap.xyz, max(ap.yxx, ap.zzy));
    return mix(vec3(0.0),sign(p), sel);
}

mat3x3 rotX(float a) {
    float s = sin(a); float c = cos(a);
    return mat3x3(1.0,0.0,0.0,0.0,c,-s,0.0,s,c);
}

mat3x3 rotY(float a) {
    float s = sin(a); float c = cos(a);
    return mat3x3(c,0.0,s,0.0,1.0,0.0,-s,0.0,c);
}

mat3x3 rotZ(float a) {
    float s = sin(a); float c = cos(a);
    return mat3x3(c,-s,0.0,s,c,0.0,0.0,0.0,1.0);
}

vec3 traceCube(vec2 uv, vec3 rot, vec3 cd, vec3 bg) {
    vec3 rayDir = vec3(0.0, 0.0, -1.0);
    vec3 rayPos = vec3(uv*1.7, 2.0);
    
    mat3x3 r = rotX(rot.x) * rotY(rot.y) * rotZ(rot.z);
    mat3x3 invr = inverse(r);
    rayDir *= r;
    rayPos *= r;

    vec2 t = intersectUnitBox(rayPos, rayDir);
    
    vec3 col;
    if ( t.x < t.y ) {
        // hit
        cd += vec3(.05,.05,.05);
        vec3 hitp = rayPos + t.x * rayDir;
        vec3 n = normalUnitBox(hitp) * invr;
        const vec3 l0 = normalize(vec3(.1,.3,1.0));
        const vec3 l1 = normalize(vec3(-.6,-.3,.2));
        
        vec3 l = vec3(1.0) * max(dot(l0,n), 0.0);
        l += vec3(.2,.2,.7) * max(dot(l1,n), 0.0);
        
        col = cd * l;
    } else {
        col = vec3(0.0);
    }
    
    return col;
}


void main(  )
{
    // Normalized pixel coordinates (from -1 to 1)
    vec2 uv = coord / dimensions.xy;
    float aspect = dimensions.x / dimensions.y;
    uv = uv * 2.0 - 1.0;
    uv.x *= aspect;
    
    float gridsize = dimensions.x / 140.0;
    gridsize = gridsize/ cubeSize;
    
    vec2 uvgrid = floor(gridsize * uv) / gridsize;
    vec2 uvsub = ((uv - uvgrid) * gridsize) * 2.0 - 1.0;
    
    vec3 bg = textureLod(tex, coord / dimensions.xy, 9.0 ).xyz;
    vec3 cd = textureLod(tex, uvgrid * vec2(.5/aspect, .5) + .5, 4.0 ).xyz;
    vec3 rot = cd * 3.0;
    rot.z += time * .4;

    vec3 col = traceCube(uvsub, rot, pow(cd, vec3(2.2)), pow(bg, vec3(2.2)));

    // Output to screen
    outputColor = vec4(pow(col,vec3(1.0/2.2)),1.0);
}


