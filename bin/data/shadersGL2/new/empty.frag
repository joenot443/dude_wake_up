#version 120

uniform sampler2DRect tex;
varying vec2 coord;

void main()
{
    vec4 tex_color=texture2DRect(tex, coord);
    gl_FragColor = vec4(tex_color.x, tex_color.y, 0.0, 1.0);
}
