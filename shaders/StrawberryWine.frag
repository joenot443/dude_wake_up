#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

void main(  )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = coord/dimensions.xy;

    // Time varying pixel color
    vec3 col = texture(tex, uv).rgb;
    // col.r = textureLod(tex, uv, 5.0).r;
    
    col.gb = mix(vec2(0.5), col.gb, smoothstep(0.0, 0.5, abs(col.gb - vec2(0.5))));
    // col.gb = mix(vec2(0.5), col.gb, smoothstep(0.0, 1.0, col.gb));
    col.gb = 0.5 * (col.gb + col.bg);
    
    // col.rgb = max(col.rgb, col.rgb - mod(col.rgb, vec3(0.25)) + 0.125);

    // Output to screen
    outputColor = vec4(col,1.0);
}