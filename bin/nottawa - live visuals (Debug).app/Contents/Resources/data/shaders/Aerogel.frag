#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;


// super cheap volumetric display based on density continuum.

void main()
{
    float att,a,r,p,g,e,b,z,x,mx,mz,my,yy,xx,zz,y,rr,sy,tr,cl,l=42.;       // l >= 2 quality
    e = 3.;                                                                // z visibility
    a =time/5.;
    mz = cos(a+3.14)*3.+time*5.;                                          // z movement
    mx = sin(a+3.14)*3.+time;                                             // x movement
    my = time/3.;                                                         // y movement
    
    for(float f=0.;f<l;f+=1.){
    zz = (1.+sin(xx+yy*917.))+(sin(yy+zz*516.))*e+e;                       // akward noise
    
                 
    xx = (coord.x/dimensions.x-0.5)*                                  // coords -0.5 to 0.5
         (dimensions.x/dimensions.y)*zz;                                 // aspect r.
    yy = (coord.y/dimensions.y-0.5)*zz;
      
    // y rot
    z =  cos(a)*(zz)-sin(a)*(xx);
    x =  sin(a)*(zz)+cos(a)*(xx);
    y =  yy;
   
    att=(l*6.);                                                            //z brightness attenuation.
   
    x+=mx;y+=my;z+=mz;
    
    // universe gen.
    sy = (float((int(x*256.)>>4&0x75)^
         (int(y*256.)>>5&0x99)^
         (int(z*256.)>>6&0xfc))/30.);
    // color distrib.
    g += float(sy>4.&&sy<6.&&f>0.)*sy/att;
    r += float(sy>5.&&sy<7.&&f>0.)*sy/att;
    b += float(sy>6.&&sy<8.&&f>0.)*sy/att;
    }
    // Out
    outputColor = vec4(r,g,b,1.0);
}
