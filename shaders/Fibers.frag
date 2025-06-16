#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

// YouTube video of livecoding. ライブコーディングのキャプチャ
// https://youtu.be/xAZE95dak04

// Corrected normal vector. 法線ベクトルを修正済み
// Other code changes. 他にもコードの変更あり

// 乱数
#define hash(x) fract(sin(x) * 5723.2622)

const float PI = acos(-1.); // 円周率
const float PI2 = PI * 2.;
const float BPM = 138.;

// If you have a strong GPU, make it bigger. GPUの処理能力に余裕がある場合、大きな値にしてください
const int nS = 2; // Number of samples. モーションブラーとDOF（被写界深度）のサンプル数

const float width = 0.03; // Width of the fiber. 円柱の半径

// Rotation matrix in two dimensions. 2Dの回転行列
mat2 rotate2D(float a) {
  float s = sin(a);
  float c = cos(a);
  return mat2(c, s, -s, c);
}

// 2Dの乱数
float hash12(vec2 p) {
  float v = dot(p, vec2(1.8672, 1.3723));
  return hash(v);
}

// 1D perlin noise. 1Dのパーリンノイズ
float perlin1d(float x) {
  float i = floor(x);
  float f = x - i; // fract(x)
  float u = f * f * (3. - 2. * f);
  return mix(f * (hash(i) * 2. - 1.), (f - 1.) * (hash(i + 1.) * 2. - 1.), u); // from -0.5 to 0.5
}

// 繊維の形（0なら直線）
float fiberShape(float x, float time) {
  float s = 0.03;
  float curve = smoothstep(0.25 - s, 0.25 + s, abs(fract(time * 0.11) - 0.5));
  return perlin1d(x) * (1. - width * 2.) * curve;
}

// Smooth stair-like noise. 滑らかな階段状のノイズ
float stepNoise(float x, float n) {
  float i = floor(x);
  //float s = 0.1;
  float s = 0.15;
  float u = smoothstep(0.5 - s, 0.5 + s, fract(x));
  return mix(floor(hash(i) * n), floor(hash(i + 1.) * n), u); // from 0. to n - 1.
}

void main(  )
{
  // Normalization of coordinates. 画面上の座標の正規化
  vec2 uv = (coord * 2. - dimensions.xy) / min(dimensions.x, dimensions.y);
  vec3 col = vec3(0);
  
  // Sampling for Motion blur and DOF. モーションブラーとDOFのサンプリング
  for(float j = 0.; j < float(nS); j++) {
    float time = time;
    vec2 seed = coord.xy + fract(time) * 500. + j * sqrt(983.);
    
    // Motion blur. モーションブラー
    time += hash12(seed) * 0.05;
    
    float Time = time * BPM / 60. * 0.5; // BPMで補正された時間
    
    vec3 ro = vec3(0, 1, -Time); // カメラの座標（レイの原点）
    vec3 ta = vec3(0, -0.5, -Time - 0.5); // ターゲットの座標
    
    // Camera movement. カメラを動かす
    ro.x += (stepNoise(ro.z, 5.) - 2.) * 0.5;
    ro.y += stepNoise(ro.z - 500., 5.) * 0.5;
    
    vec3 dir = normalize(ta - ro); // カメラの方向
    vec3 side = normalize(cross(dir, vec3(0, 1, 0)));
    vec3 up = normalize(cross(side, dir));
    
    float fov = 40.; // FOV
    fov += (stepNoise(ro.z - 1000., 2.) * 2. - 1.) * 20.;
    vec3 rd = normalize(uv.x * side + uv.y * up + dir / tan(fov / 360. * PI)); // レイの方向
    
    // Depth of field (DOF). 被写界深度
    float L = length(ta - ro);
    vec3 fp = ro + rd * L;
    vec3 ros = ro;
    float r = sqrt(hash12(seed * 1.1));
    float theta = hash12(seed * 1.2) * PI2;
    ros.xy += r * vec2(cos(theta), sin(theta)) * L * 0.05;
    vec3 rds = normalize(fp - ros);
    
    bool hit = false;
    float t; // Ray length to the object. オブジェクト表面まで伸ばしたレイの長さ
    vec2 p;
    vec2 ID;
    float ref;
    float angle;
    
    // Raycasting (intersection detection) to the fibers (50 planes).
    // 繊維（50枚の平面）へのレイキャスティング（交差判定）
    for(float i = 0.; i < 50.; i++) {
      t = -(ros.y + i * 0.05) / rds.y;
      p = ros.xz + t * rds.xz; // レイの先端のxz座標
      p.x += hash(i) * 500.;
      angle = i * 2.3;
      p *= rotate2D(angle);
      
      ID = vec2(i, floor(p.x)); // 繊維1本毎に異なるID
      
      p.x = fract(p.x) - 0.5;
      p.y += hash12(ID) * 500.;
      ref = fiberShape(p.y, time);
      p.x -= ref;
      
      if(t > 0. && abs(p.x) < width) {
        hit = true; // レイがオブジェクトに衝突した
        break;
      }
    }
    
    if(!hit) {
      continue;
    }
    
    // Ray hit an object. レイがオブジェクトに衝突した
    vec3 add = vec3(1);
    
    vec3 lightDir = normalize(vec3(-5, 2, -2)); // 光源の方向
    vec3 normal = normalize(vec3(p.x, sqrt(width * width - p.x * p.x), 0)); // 法線ベクトル
    
    // Modify normal vector using the rotation angle and the gradient of fiber shape.
    // 回転角度と繊維の形（勾配）を使って法線ベクトルを補正する
    float e = 1e-4;
    float grad = (fiberShape(p.y + e, time) - ref) / e;
    normal.xz *= rotate2D(-angle + atan(grad));
    
    // Shading. シェーディング
    float diff = max(dot(normal, lightDir), 0.);
    float spec = pow(max(dot(reflect(lightDir, normal), rds), 0.), 20.);
    float m = 0.5;
    float lp = 3.;
    add *= mix(diff, spec, m) * lp + 0.2;
    
    // Streaming light. 流れる光
    p.y = fract(p.y * 0.03 - time * 0.2) - 0.5;
    add += smoothstep(0.01, 0., abs(p.y)) * 3.;
    
    // Flickering light. チカチカする光
    float T = time + hash12(ID);
    add += step(hash12(ID * 1.1 + floor(T)), 0.05) * step(fract(T * 3.), 0.8) * 3.;
    
    // Black fog. 黒い霧
    add *= exp(-t * t * 0.1 - ID.x * ID.x * 0.001);
    
    col += add;
  }
  
  col /= float(nS);
  col = pow(col, vec3(1. / 2.2)); // Gamma correction. ガンマ補正
  outputColor = vec4(col, 1);
}

