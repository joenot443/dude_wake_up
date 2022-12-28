#version 150

uniform sampler2D tex;
uniform float time;
in vec2 coord;
out vec4 outputColor;

void main()
{
	vec2 uv = coord*0.8 + 0.1;
    
  uv += cos(time*vec2(6.0, 7.0) + uv*10.0)*0.02;
    
	outputColor = texture(tex, uv);
}


