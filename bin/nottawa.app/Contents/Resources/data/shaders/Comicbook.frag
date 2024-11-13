#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;


vec3 GreyScale(vec3 InCol);
vec3 Threshold(vec3 InCol);

void main(  ){
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = coord/dimensions.xy;
    
    vec2 TopLeft      = vec2( 1., 1.)/dimensions.xy;
    vec2 TopMiddle    = vec2( 0., 1.)/dimensions.xy;
    vec2 TopRight     = vec2(-1., 1.)/dimensions.xy;
    vec2 MiddleLeft   = vec2( 1., 0.)/dimensions.xy;
    vec2 MiddleMiddle = vec2( 0., 0.)/dimensions.xy;
    vec2 MiddleRight  = vec2(-1., 0.)/dimensions.xy;
    vec2 BottomLeft   = vec2( 1.,-1.)/dimensions.xy;
    vec2 BottomMiddle = vec2( 0.,-1.)/dimensions.xy;
    vec2 BottomRight  = vec2(-1.,-1.)/dimensions.xy;

    // Time varying pixel color
    vec3 col = texture(tex,uv).rgb;
    int Min;
    if(min(min(col.r,col.g),col.b)==col.r){
        Min=0;
    }else if(min(min(col.r,col.g),col.b)==col.g){
        Min=1;
    }else{
        Min=2;
    }
    
    int Max;
    if(max(max(col.r,col.g),col.b)==col.r){
        Max=0;
    }else if(max(max(col.r,col.g),col.b)==col.g){
        Max=1;
    }else{
        Max=2;
    }
    
    int Mid;
    if((Max+Min)==1){
        Mid=2;
    }else if((Max+Min)==2){
        Mid=1;
    }else{
        Mid=0;
    }
    
    float Cols[3]=float[3](
        col.r,
        col.g,
        col.b
    );
    
    float a = Cols[Max]-Cols[Min];
    float b = Cols[Mid]-Cols[Min];
    float x = Cols[Max];
    
    Cols[Max]=x;
    Cols[Min]=0.;
    Cols[Mid]=x*(b/a);
    vec3 Scol=vec3(Cols[0],Cols[1],Cols[2]);
    Scol=Scol/max(max(Scol.r,Scol.g),Scol.b);
    
    col = Threshold(vec3(1.-sqrt(pow((
               GreyScale(texture(tex,uv+TopLeft     ).rgb)* 1.+
               GreyScale(texture(tex,uv+TopMiddle   ).rgb)* 2.+
               GreyScale(texture(tex,uv+TopRight    ).rgb)* 1.+
               GreyScale(texture(tex,uv+MiddleLeft  ).rgb)* 0.+
               GreyScale(texture(tex,uv+MiddleMiddle).rgb)* 0.+
               GreyScale(texture(tex,uv+MiddleRight ).rgb)* 0.+
               GreyScale(texture(tex,uv+BottomLeft  ).rgb)*-1.+
               GreyScale(texture(tex,uv+BottomMiddle).rgb)*-2.+
               GreyScale(texture(tex,uv+BottomRight ).rgb)*-1.
                ).r,2.)+
               pow((
               GreyScale(texture(tex,uv+TopLeft     ).rgb)* 1.+
               GreyScale(texture(tex,uv+TopMiddle   ).rgb)* 0.+
               GreyScale(texture(tex,uv+TopRight    ).rgb)*-1.+
               GreyScale(texture(tex,uv+MiddleLeft  ).rgb)* 2.+
               GreyScale(texture(tex,uv+MiddleMiddle).rgb)* 0.+
               GreyScale(texture(tex,uv+MiddleRight ).rgb)*-2.+
               GreyScale(texture(tex,uv+BottomLeft  ).rgb)* 1.+
               GreyScale(texture(tex,uv+BottomMiddle).rgb)* 0.+
               GreyScale(texture(tex,uv+BottomRight ).rgb)*-1.
               ).r,2.))))*
               Scol;
        

    // Output to screen
    outputColor = vec4(col,1.);
}

vec3 GreyScale(vec3 InCol){
    return vec3((InCol.r+InCol.g+InCol.b)/3.);
}

vec3 Threshold(vec3 InCol){
    return (1.-vec3(0.,0.,0.))*float(InCol.r>0.8);
}
