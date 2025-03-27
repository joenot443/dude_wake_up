#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

//Display the buffer
void main(  )
{
	vec2 uv = coord.xy / dimensions.xy;
	outputColor = texture(tex, uv);
}

