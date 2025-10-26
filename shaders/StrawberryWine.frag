#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
uniform float colorDesaturation;
uniform float channelMix;
uniform float redIntensity;
in vec2 coord;
out vec4 outputColor;

void main(  )
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 uv = coord/dimensions.xy;

    // Time varying pixel color
    vec3 col = texture(tex, uv).rgb;
    // col.r = textureLod(tex, uv, 5.0).r;

    // Apply color desaturation with controllable threshold
    col.gb = mix(vec2(0.5), col.gb, smoothstep(0.0, colorDesaturation, abs(col.gb - vec2(0.5))));
    // col.gb = mix(vec2(0.5), col.gb, smoothstep(0.0, 1.0, col.gb));

    // Apply channel mixing with controllable blend amount
    col.gb = mix(col.gb, 0.5 * (col.gb + col.bg), channelMix);

    // Apply red intensity multiplier
    col.r *= redIntensity;

    // col.rgb = max(col.rgb, col.rgb - mod(col.rgb, vec3(0.25)) + 0.125);

    // Output to screen
    outputColor = vec4(col,1.0);
}