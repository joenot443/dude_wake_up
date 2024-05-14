#version 150

uniform sampler2D tex_0;
uniform sampler2D tex_1;
uniform sampler2D tex_2;
uniform sampler2D tex_3;
uniform sampler2D tex_4;
uniform sampler2D tex_5;
uniform sampler2D tex_6;
uniform sampler2D tex_7;
uniform sampler2D tex_8;
uniform sampler2D tex_9;

uniform float mix_0;
uniform float mix_1;
uniform float mix_2;
uniform float mix_3;
uniform float mix_4;
uniform float mix_5;
uniform float mix_6;
uniform float mix_7;
uniform float mix_8;
uniform float mix_9;


in vec2 coord;
out vec4 outputColor;

void main()
{
  vec4 accumulatedColor = vec4(0.0);

  if (mix_0 > 0.01) {
    vec4 tex_color = texture(tex_0, coord);
    if (tex_color.a > 0.1) {
      accumulatedColor += tex_color * mix_0;
    }
  }

  if (mix_1 > 0.01) {
    vec4 tex_color = texture(tex_1, coord);
    if (tex_color.a > 0.1) {
      accumulatedColor += tex_color * mix_1;
    }
  }

  if (mix_2 > 0.01) {
    vec4 tex_color = texture(tex_2, coord);
    if (tex_color.a > 0.1) {
      accumulatedColor += tex_color * mix_2;
    }
  }

  if (mix_3 > 0.01) {
    vec4 tex_color = texture(tex_3, coord);
    if (tex_color.a > 0.1) {
      accumulatedColor += tex_color * mix_3;
    }
  }

  if (mix_4 > 0.01) {
    vec4 tex_color = texture(tex_4, coord);
    if (tex_color.a > 0.1) {
      accumulatedColor += tex_color * mix_4;
    }
  }

  if (mix_5 > 0.01) {
    vec4 tex_color = texture(tex_5, coord);
    if (tex_color.a > 0.1) {
      accumulatedColor += tex_color * mix_5;
    }
  }

  if (mix_6 > 0.01) {
    vec4 tex_color = texture(tex_6, coord);
    if (tex_color.a > 0.1) {
      accumulatedColor += tex_color * mix_6;
    }
  }

  if (mix_7 > 0.01) {
    vec4 tex_color = texture(tex_7, coord);
    if (tex_color.a > 0.1) {
      accumulatedColor += tex_color * mix_7;
    }
  }

  if (mix_8 > 0.01) {
    vec4 tex_color = texture(tex_8, coord);
    if (tex_color.a > 0.1) {
      accumulatedColor += tex_color * mix_8;
    }
  }

  if (mix_9 > 0.01) {
    vec4 tex_color = texture(tex_9, coord);
    if (tex_color.a > 0.1) {
      accumulatedColor += tex_color * mix_9;
    }
  }
  
  outputColor = accumulatedColor;
}
