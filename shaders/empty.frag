#version 150

uniform sampler2DRect tex;
uniform vec2 coord;
out vec4 outputColor;

void main()
{
    vec4 tex_color=texture(tex, coord);
    gl_FragColor = vec4(tex_color.x, tex_color.y, tex_color.z, 1.0);
}
