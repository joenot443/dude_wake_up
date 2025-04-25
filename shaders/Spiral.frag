#version 150

uniform sampler2D tex;          // optional background
uniform vec2  dimensions;       // (width, height) in pixels
uniform float time;             // seconds since start
uniform float speed;            // user-controlled angular speed
uniform vec3  mainColor;        // colour of the bright spiral band
uniform vec3  secondaryColor;   // colour of the dark band / gaps
uniform float size;             // global scale (>0)

in  vec2 coord;                 // 0-1 UV from the pipeline
out vec4 outputColor;

const float PI = 3.141592654;

void main()
{
  /* centred, aspect-corrected UV - matches Shadertoy example logic */
  vec2 fragCoord = coord * dimensions;
  vec2 uv = (fragCoord - 0.5 * dimensions) / dimensions.y;   // keep spiral circular
  
  /* radius & angle */
  float r     = length(uv) / max(size, 1e-4);                // scale via `size`
  float theta = atan(uv.y, uv.x);
  
  /* spiral equation (from your reference) */
  float stripe = fract( 2.5 * theta / PI                    // angular term
                       + 7.0 * pow(r, 0.4)                   // radial term
                       - 2.5 * time * speed );               // animation
  
  /* choose colour */
  vec3 spiralCol = (stripe < 0.5) ? mainColor : secondaryColor;
  
  outputColor = vec4(spiralCol, 1.0);
}
