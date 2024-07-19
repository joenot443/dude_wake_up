#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float amount;
in vec2 coord;
out vec4 outputColor;


vec2 f_o_rotd(vec2 o, float n_ang){
  return vec2(
              cos(n_ang)*o.x - sin(n_ang)*o.y,
              sin(n_ang)*o.x + cos(n_ang)*o.y
              );
}
#define n_tau 6.2831

void main(  )
{
  vec2 o_trn = (coord.xy)/ dimensions.xy;
  vec2 o_trn_m = vec2(amount, amount);
  vec2 o_trn_off = (coord.xy-dimensions.xy*.5)/ dimensions.y;
  

  float n_dcntr = length(o_trn_off);
  n_dcntr *= max(dimensions.x / dimensions.y, dimensions.y / dimensions.x);
  n_dcntr = pow(n_dcntr, 1./3.);
  n_dcntr = clamp(n_dcntr, 0., 1.);
  n_dcntr = 1.-n_dcntr;
  
  o_trn -= .5;
  o_trn = f_o_rotd(o_trn, n_dcntr * n_tau * o_trn_m.x);
  o_trn +=.5;
  vec4 o_col_webcam = texture(tex, fract(o_trn));
  
  outputColor = vec4(o_col_webcam);
}
