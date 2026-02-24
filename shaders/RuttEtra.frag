#version 150

uniform sampler2D tex;
uniform vec2 dimensions;
uniform float time;
in vec2 coord;
out vec4 outputColor;

uniform float lines;
uniform float lineWidth;
uniform float extrusion;
uniform float yScale;
uniform float brightness;
uniform float fillOpacity;

float getLuma(vec3 color) {
    return dot(color, vec3(0.299, 0.587, 0.114));
}

void main()
{
    vec2 uv = coord / dimensions;
    // Flip Y so line 0 is at bottom
    uv.y = 1.0 - uv.y;

    vec3 finalColor = vec3(0.0);

    // Back-to-front (Painter's Algorithm)
    for (float i = lines; i >= 0.0; i -= 1.0) {

        float normIndex = i / lines;

        // Sample the texture at this scanline row
        vec2 sampleUV = vec2(uv.x, 1.0 - normIndex);
        vec3 texColor = texture(tex, sampleUV).rgb;
        float luma = getLuma(texColor);

        // Base Y position for this line + luminance displacement
        float lineY = normIndex * yScale;
        lineY += luma * extrusion;

        // Distance from pixel to this line
        float dist = uv.y - lineY;

        // Anti-aliased line using screen-space derivatives
        float aaWidth = fwidth(dist);
        aaWidth = max(aaWidth, 0.0005);

        float lineAlpha = 1.0 - smoothstep(0.0, aaWidth * lineWidth, abs(dist));

        vec3 lineColor = texColor * brightness;

        // Fill: if pixel is below this line, occlude with black
        float fillAlpha = 1.0 - smoothstep(0.0, aaWidth, dist);
        if (fillAlpha > 0.5) {
            finalColor = mix(finalColor, vec3(0.0), fillOpacity);
        }

        // Draw line on top
        finalColor = mix(finalColor, lineColor, lineAlpha);
    }

    outputColor = vec4(finalColor, 1.0);
}
