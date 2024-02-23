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

void main()
{
    // 画像の解像度を取得
    vec2 resolution = dimensions.xy;

    // ピクセルのUV座標を取得
    vec2 uv = coord / resolution;

    // テクスチャからピクセルカラーを取得
    vec3 originalColor = texture(tex, uv).rgb;
    // YCbCr変換
    float Y = 0.299 * originalColor.r + 0.587 * originalColor.g + 0.114 * originalColor.b;
    float Cb = 0.564 * (originalColor.b - Y);
    float Cr = 0.713 * (originalColor.r - Y);
    
    vec3 diff = texture(tex, uv+vec2(1.0/resolution.x,0)).rgb - texture(tex, uv).rgb;
    float diffY = 0.299 * diff.r + 0.587 * diff.g + 0.114 * diff.b;

    // 色相信号にノイズを追加
    float hueNoise1 = noise(vec2(uv.r*50.0,uv.g*50.0),time*5.5383) * 0.1-0.05; // ノイズの範囲を調整
    float hueNoise2 = noise(vec2(uv.r*50.0,uv.g*50.0),time*2.2546) * 0.1-0.05; // ノイズの範囲を調整
    float hueNoise3 = noise(vec2(uv.r*5.0,uv.g*30.0),time*6.4623) * 0.15-0.075; // ノイズの範囲を調整
    float hueNoise4 = noise(vec2(uv.r*5.0,uv.g*30.0),time*8.4352) * 0.15-0.075; // ノイズの範囲を調整
    // 色相情報にノイズを加える
    Cr += hueNoise1;
    Cb += hueNoise2;
    Cr += hueNoise3;
    Cb += hueNoise4;
    
    //エンファシス
    Cb *= 1.5;
    Cr *= 1.5;
    Y  *= 0.75;
    
    //クロスカラー
    Cb = Cb + cos(coord.x) * diffY;
    Cr = Cr + sin(coord.x) * diffY;
    
    // YCbCrからRGBに戻す
    float R = Y + 1.403 * Cr;
    float G = Y - 0.344 * Cb - 0.714 * Cr;
    float B = Y + 1.773 * Cb;
    
    outputColor = vec4(R,G,B,1.0);
}

