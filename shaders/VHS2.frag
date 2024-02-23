#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

float random(vec2 st, float seed){
    return fract(sin(seed+dot(st.xy, vec2(12.9898,78.233)))* 43758.5453123);
}


//参考:https://thebookofshaders.com/11/
float noise (in vec2 st, float seed) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    float a = random(i,seed);
    float b = random(i + vec2(1.0, 0.0),seed);
    float c = random(i + vec2(0.0, 1.0),seed);
    float d = random(i + vec2(1.0, 1.0),seed);

    vec2 u = f*f*(3.0-2.0*f);

    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

void main(  )
{
    // 画像の解像度を取得
    vec2 resolution = dimensions.xy;

    // ピクセルのUV座標を取得
    vec2 uv = coord / resolution;

    // テクスチャからピクセルカラーを取得
    // 畳み込みフィルタのカーネルを定義
    float kernel[12];
    kernel[0] = -4.0 / 9.0;
    kernel[1] = -3.0 / 9.0;
    kernel[2] = -2.0 / 9.0;
    kernel[3] = -1.0 / 9.0;
    kernel[4] = 0.0 / 9.0;
    kernel[5] = 1.0 / 9.0;
    kernel[6] = 2.0 / 9.0;
    kernel[7] = 3.0 / 9.0;
    kernel[8] = 4.0 / 9.0;
    kernel[9] = 4.0 / 9.0;
    kernel[10] = 4.0 / 9.0;
    kernel[11] = 4.0 / 9.0;

    vec3 convolvedColor = vec3(0.0);

    // hsyncジッターを計算
    float hsync = floor(noise(vec2(uv.y*50.0,0.0),time*5.5383)*3.0);
  
    // 畳み込み演算を実行
    for (int i = -4; i <= 4; i++) {
        vec2 offset = vec2(float(i), 0);
        convolvedColor += texture(tex, uv + (offset + vec2(hsync,0.0)) / resolution).rgb * kernel[(i + 6)];
    }
    // テクスチャからピクセルカラーを取得
    vec3 originalColor = convolvedColor;

    outputColor = vec4(originalColor,1.0);
}
