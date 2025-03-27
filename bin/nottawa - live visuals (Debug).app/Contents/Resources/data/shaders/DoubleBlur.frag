#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float amount;
uniform int direction; // 0 for horizontal, 1 for vertical
in vec2 coord;
out vec4 outputColor;

void main() {
    vec2 tex_offset;
    if (direction == 0) {
        tex_offset = vec2(1.0 / dimensions.x, 0.0); // horizontal offset based on texture width
    } else {
        tex_offset = vec2(0.0, 1.0 / dimensions.y); // vertical offset based on texture height
    }

    float blurRadius = amount; // you can adjust this or pass it as a uniform

    vec3 result = texture(tex, coord).rgb * 0.227027;
    result += texture(tex, coord + tex_offset * blurRadius).rgb * 0.1945946;
    result += texture(tex, coord - tex_offset * blurRadius).rgb * 0.1945946;
    result += texture(tex, coord + tex_offset * 2.0 * blurRadius).rgb * 0.1216216;
    result += texture(tex, coord - tex_offset * 2.0 * blurRadius).rgb * 0.1216216;
    result += texture(tex, coord + tex_offset * 3.0 * blurRadius).rgb * 0.054054;
    result += texture(tex, coord - tex_offset * 3.0 * blurRadius).rgb * 0.054054;

    outputColor = vec4(result, 1.0);
}
