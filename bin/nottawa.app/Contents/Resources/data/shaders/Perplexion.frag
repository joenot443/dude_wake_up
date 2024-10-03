#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float alpha;
uniform float beta;
uniform float gamma;
uniform float speed;
in vec2 coord;
out vec4 outputColor;


#define rot(x) mat2(cos(x+vec4(0,11,33,0)))


//formula for creating colors;
#define H(h)  (  cos(  h/5. + 4.*vec3(1,3,2) * alpha )*.8  )


//formula for mapping scale factor
#define M(c)  log(c)

#define R dimensions

#define ex exp(w.x)
#define cy cos(w.y)
#define sy sin(w.y)
#define cz cos(w.z)
#define sz sin(w.z)
#define cw cos(w.w)
#define sw sin(w.w)


void main() {
  
  outputColor =  vec4(0.5);
  
  vec3 c=vec3(0);
  vec4 rd = normalize( vec4(coord-.5*R.xy, .6*R.y, 0.));
  
  float dotp, totdist=0., tt=time * speed, t=0.;
  
  for (float i=0.; i<20.; i++) {
    
    vec4 p = vec4( rd*totdist) ;
    
    p.z  += 2.;
    p.x += 2.;
    p.y -= 5.;
    
    p.xz *= rot( 1.4*(1.+.2*sin(tt/9.) ) ) ;
    p.xy *= rot( -.6 * (1.+.2*sin(tt/11.)) ) ;
    
    vec4 w=vec4(0,0,3.*sin(tt/11.),3.*cos(tt/11.)) , dz, z;
    
    
    for (float j=0.; j<4.; j++) {
      
      z = vec4(
               ex*(cy*cz*cw - sy*sz*sw),
               ex*(sy*cz*cw + cy*sz*sw),
               ex*(cy*sz*cw - sy*cz*sw),
               ex*(sy*sz*cw + cy*cz*sw)
               );
      
      //iterated derivative here instead of just scale factor
      dz = vec4(z.x*dz.x-z.y*dz.y-z.z*dz.z-z.w*dz.w,
                z.x*dz.y+z.y*dz.x+z.z*dz.w-z.w*dz.z,
                z.x*dz.z-z.y*dz.w+z.z*dz.x+z.w*dz.y,
                z.x*dz.w+z.y*dz.z-z.z*dz.y+z.w*dz.x);
      
      dz.x += 1. ;
      
      dotp = max(1./dot(z.xy,z.xy),.01);
      
      dz *=  dotp;
      
      w = z *dotp + p ;
      
    }
    
    float ddz = clamp(dot(dz,dz),1e-3,1e5);
    float ddw = clamp(dot(w,w),1e-3,1e5);
    
    float dist = pow(ddw/ddz,.001) * log(ddw);
    
    float stepsize = dist/2.;
    totdist += stepsize;
    
    c +=
    
    H( log(ddz) ) * exp(-i*i*dist*dist*1e-3);
  }
  
  c = 1. - exp(-c*c);
  outputColor =  vec4(c,1.0);
}
