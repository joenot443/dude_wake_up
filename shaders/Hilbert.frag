#version 150

uniform mat4 modelViewProjectionMatrix;

// Standard vertex coordinates
in vec4 position;
// Uv coordinates
in vec2 texcoord;
uniform vec2 dimensions;
uniform sampler2D tex;
uniform float time;

out vec2 coord;


#define Res vec2(dimensions.xy)
#define Res0 vec2(textureSize(tex,0))
#define Res1 vec2(textureSize(tex,0))
#define PI  3.14159265359
#define PI2 6.28318530718
#define PIH 1.57079632679

#define ROT(ang) mat2(cos(ang+vec2(0.,PI*.5)),sin(ang+vec2(0.,PI*.5)))

#define PatternRot (time*.1)
#define randness .0
#define iMouse vec2(0.5,0.5)

vec4 getRand(int idx)
{
    return texelFetch(tex, ivec2(idx%256,(idx/256)%256),0);
}

mat2 getHilbTrans(int idx)
{
//  mirr, ang  (4 points)
//    -1,-90
//     1,  0
//     1,  0
//    -1, 90
    float a=float( (((idx/2)&1)*2-1)*(((idx+3)/2)&1) ); // (-1,-1,1,1)*(1,0,0,1)=(-1,0,0,1)
    float mir=-abs(a)*2.+1.;
    float ang=a*PIH;
    vec2 cs=cos(ang-vec2(0,PIH));
    return mat2(cs,cs.yx*vec2(-1,1))*mat2(mir,0,0,1);
}
    
vec2 getHilbPoint(int idx)
{
    return vec2((ivec2(idx,idx+1)/2)&1)-.5;
}

int getHilbIdx03(vec2 sc)
{
    // get quadrant index
    // 1 2
    // 0 3
    return int(atan(sc.x,sc.y)/PIH+2.);
}

float dDirLine(vec3 p, vec3 c, vec3 dir, float l)
{
    p-=c;
    dir=normalize(dir);
    float dp=dot(p,dir);
    return max(max(length(p-dp*dir),-dp),dp-l);
}

float dLine(vec3 p, vec3 p1, vec3 p2) { return dDirLine(p,p1,normalize(p2-p1),length(p2-p1)); }

float dLine(vec2 p, vec2 p1, vec2 p2) { return dLine(vec3(p,0),vec3(p1,0),vec3(p2,0)); }

// get hilbert point of index idx with lev subdivision levels
vec2 getHilbPoint(int idx,int lev)
{
    vec2 p=vec2(0);
    float scale=1.;
    mat2 m=mat2(1,0,0,1);
    for(int l=0;l<lev;l++)
    {
        int il=(idx>>((lev-1-l)*2))&3;
        p+=m*(getHilbPoint(il)*scale);
        m=m*getHilbTrans(il);
        scale*=.5;
    }
    return p;
}

vec2 getHilbPointF(int idx,float lev)
{
    vec2 p=vec2(0);
    float scale=1.;
    mat2 m=mat2(1,0,0,1);
    int maxLev=int(floor(lev)+1.)-1;
    maxLev=min(maxLev,10);
    for(int l=0;l<=maxLev;l++)
    {
        int il=(idx>>((maxLev-l)*2))&3;
        float levFade=1.;
        if(l==maxLev) levFade=fract(lev);
        vec2 offs=vec2(0.);
        if(l==maxLev-1) offs=(getRand(idx).xy-.5)*.2*randness;
        if(l==maxLev) offs=(getRand(idx).xy-.5)*.2*(1.-fract(lev))*randness;
        p+=m*((getHilbPoint(il)+offs)*scale)*levFade;
        m=m*getHilbTrans(il);
        scale*=.5;
    }
    return p;
}

int getClosestHilbIdx(vec2 p,int lev)
{
    float scale=1.;
    int idxAll=0;
    for(int l=0;l<lev;l++)
    {
        int idx=getHilbIdx03(p);
        idxAll=idxAll*4+idx;
        mat2 m=getHilbTrans(idx);
        p=m*(p-getHilbPoint(idx)*scale);
        scale*=.5;
    }
    return idxAll;
}

float hilbert2d(vec2 pos, int level)
{
    float d=10000.;
    for(int i=0; i<int(iMouse.x/10.); i++)
    {
        vec2 p1=getHilbPoint(i,level);
        vec2 p2=getHilbPoint(i+1,level);
        d=min(d,dLine(pos,p1,p2));
    }
    return d;
}


vec2 getInterPoint(vec2 p[11], int num, float fact)
{
    fact=clamp(fact,0.,1.);
    float idxf=fact*(float(num)-.001);
    int fi=int(idxf);
    return mix(p[fi],p[min(fi+1,num-1)],fract(idxf));
}

vec2 getSmoothInterPoint(vec2 p[11], int num, float fact, float w)
{
    vec2 p1=getInterPoint(p, num, fact-w);
    vec2 p2=getInterPoint(p, num, fact+w);
    return mix(p1,p2,.5);
}

float hilbDistF(vec2 p, float lev)
{
    vec2 h[11];
    int idx0=getClosestHilbIdx(p,int(lev+1.));
    for(int i=0;i<=8;i++)
    {
        h[i]=getHilbPointF(idx0+i-4,lev);
    }
    float d=10000.;
    //d=dSpline(h,8);
    for(int i=0;i<8;i++)
    {
        d=min(d,dLine(p,h[i],h[i+1]));
    }
    return d;
}

float hilbPat(vec2 p, float lev)
{
    float sc=1.*exp2(lev);
    return hilbDistF(p, lev);
}

#define LineWidth (1.*pow(Res.x/700.,.3)*(.75+iMouse.y/Res.y))
#define Inverse 0.

#define VidTex tex

float WhiteVign=1.;

vec4 getCol(vec2 c)
{
    vec2 uv=(c-.5*Res)*min(Res0.x/Res.x,Res0.y/Res.y)/Res0+.5;
    //return textureLod(VidTex,c/Res,0.);
    vec4 col=vec4(0);
    float sum=0.;
    for(int i=0;i<4;i++)
    {
        float f=exp2(-float(i)*1.);
        col+=f*textureLod(VidTex,uv,.5+float(i)*1.2);
        sum+=f;
    }
    col/=sum;
    float l=length(c/Res-.5);
  //col*=1.1;
    //col-=.1;
    col=mix(col,vec4(1),clamp(WhiteVign*l*l,0.,1.));
    return clamp(col,0.,.9);
}

float getVal(vec2 c)
{
    //return dot(getCol(c).xyz,vec3(.3,.5,.2));
    return dot(getCol(c).xyz,vec3(.3333));
}

void main()
{
    float fitRes=min(Res.x,Res.y);
    vec2 sc=(coord-Res*.5)/fitRes*2.;
    sc=ROT(PatternRot)*sc;
    
    float br=getVal(coord);
    br=clamp(br,0.,1.);
    //br*=.9;
    br=mix(br,1.-br,Inverse);
    //sc+=(texture(iChannel1,sc*.2).xy-.5)*.006;
    //br=.1+.9*br;
    float scale=1.;
    //scale*=exp(-iMouseData.z/3000.)/sqrt(Res.x/600.);
    scale*=exp(-iMouse.x/Res.x)/sqrt(Res.x/600.);
    scale*=max(.8,.01)*(fitRes/Res.x);
    float d=hilbPat(sc, log2(br*.5*Res.x*scale));
    
    float s=2./(fitRes*length(vec2(fwidth(sc.y),fwidth(sc.x))));
    
    d=hilbDistF(sc, log2((1.-br)*1.*Res.x*scale))*fitRes/2.;
    //d=hilbPat(sc, iMouse.x/Res.x*10.);
    
    float w=1.4*fwidth(d);
    outputColor.xyz = vec3(0)+exp(-(d-1.+br)*(d-1.+br)/w/w);
    float th=LineWidth-.35;
    //float p=smoothstep(th-.7,th+.7,d);
    float p=clamp(d*s*1.4+1.-LineWidth,0.,1.);
    p=mix(p,1.-p,Inverse);
    outputColor = vec4(p.r, p.g, p.b, 1.0);
}

