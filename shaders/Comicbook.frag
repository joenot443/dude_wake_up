#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float edgeThreshold;
uniform float edgeWidth;
uniform float edgeSoftness;
uniform float saturation;
uniform float posterize;
uniform float colorBleed;
in vec2 coord;
out vec4 outputColor;


float GreyScale(vec3 InCol);

void main() {
    vec2 uv = coord/dimensions.xy;

    // Per-channel UV offset for "print misregistration" color bleed.
    // R shifts one way, B the other, G stays put.
    vec2 bleedOffset = vec2(colorBleed, 0.0) / dimensions.xy * 4.0;
    vec3 col;
    col.r = texture(tex, uv + bleedOffset).r;
    col.g = texture(tex, uv).g;
    col.b = texture(tex, uv - bleedOffset).b;

    // Sobel kernel offsets, scaled by edgeWidth so the user can fatten the lines.
    vec2 px = edgeWidth / dimensions.xy;
    vec2 TopLeft      = vec2( 1.,  1.) * px;
    vec2 TopMiddle    = vec2( 0.,  1.) * px;
    vec2 TopRight     = vec2(-1.,  1.) * px;
    vec2 MiddleLeft   = vec2( 1.,  0.) * px;
    vec2 MiddleRight  = vec2(-1.,  0.) * px;
    vec2 BottomLeft   = vec2( 1., -1.) * px;
    vec2 BottomMiddle = vec2( 0., -1.) * px;
    vec2 BottomRight  = vec2(-1., -1.) * px;

    // Saturate the central color into the comic palette: keep max channel,
    // zero out the min, scale the mid by the original ratio, then normalize.
    int Min;
    if(min(min(col.r,col.g),col.b)==col.r){ Min=0; }
    else if(min(min(col.r,col.g),col.b)==col.g){ Min=1; }
    else{ Min=2; }

    int Max;
    if(max(max(col.r,col.g),col.b)==col.r){ Max=0; }
    else if(max(max(col.r,col.g),col.b)==col.g){ Max=1; }
    else{ Max=2; }

    int Mid;
    if((Max+Min)==1){ Mid=2; }
    else if((Max+Min)==2){ Mid=1; }
    else{ Mid=0; }

    float Cols[3]=float[3]( col.r, col.g, col.b );

    float a = Cols[Max]-Cols[Min];
    float b = Cols[Mid]-Cols[Min];
    float x = Cols[Max];

    Cols[Max]=x;
    Cols[Min]=0.;
    Cols[Mid]=x*(b/a);
    vec3 Scol=vec3(Cols[0],Cols[1],Cols[2]);
    Scol=Scol/max(max(Scol.r,Scol.g),Scol.b);

    // Blend natural color with the comic-saturated version.
    vec3 paletteCol = mix(col, Scol, saturation);

    // Posterize: 0 = effectively no quantization (256 levels), 1 = harsh (2 levels).
    // Below 8-bit precision the banding is invisible, so default 0 introduces no flicker.
    float levels = mix(256.0, 2.0, posterize);
    paletteCol = floor(paletteCol * levels) / levels;

    // Sobel edge magnitude on grayscale luminance.
    float gx =
        GreyScale(texture(tex,uv+TopLeft     ).rgb)* 1.+
        GreyScale(texture(tex,uv+TopMiddle   ).rgb)* 2.+
        GreyScale(texture(tex,uv+TopRight    ).rgb)* 1.+
        GreyScale(texture(tex,uv+BottomLeft  ).rgb)*-1.+
        GreyScale(texture(tex,uv+BottomMiddle).rgb)*-2.+
        GreyScale(texture(tex,uv+BottomRight ).rgb)*-1.;

    float gy =
        GreyScale(texture(tex,uv+TopLeft     ).rgb)* 1.+
        GreyScale(texture(tex,uv+TopRight    ).rgb)*-1.+
        GreyScale(texture(tex,uv+MiddleLeft  ).rgb)* 2.+
        GreyScale(texture(tex,uv+MiddleRight ).rgb)*-2.+
        GreyScale(texture(tex,uv+BottomLeft  ).rgb)* 1.+
        GreyScale(texture(tex,uv+BottomRight ).rgb)*-1.;

    float edgeMag = sqrt(gx*gx + gy*gy);
    float lineInput = 1.0 - edgeMag; // 1 = no edge, 0 = strong edge

    // edgeSoftness=0 reproduces the original binary threshold via smoothstep with
    // a vanishing band. Larger softness = painterly transition between ink and color.
    float edgeMask = smoothstep(
        edgeThreshold - edgeSoftness,
        edgeThreshold + edgeSoftness + 1e-5,
        lineInput
    );

    outputColor = vec4(edgeMask * paletteCol, 1.);
}

float GreyScale(vec3 InCol){
    return (InCol.r+InCol.g+InCol.b)/3.;
}
