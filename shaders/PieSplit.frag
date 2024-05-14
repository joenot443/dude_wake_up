#version 150

// Uniforms for each texture
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;
uniform sampler2D tex4;
uniform sampler2D tex5;
uniform sampler2D tex6;
uniform sampler2D tex7;
uniform sampler2D tex8;
uniform sampler2D tex9;
uniform sampler2D tex10;

// Active flags for each texture
uniform int tex1_active;
uniform int tex2_active;
uniform int tex3_active;
uniform int tex4_active;
uniform int tex5_active;
uniform int tex6_active;
uniform int tex7_active;
uniform int tex8_active;
uniform int tex9_active;
uniform int tex10_active;

uniform vec2 dimensions;
uniform float time;

in vec2 coord;
out vec4 outputColor;

const float PI = 3.14159265359;

void main() {
    vec2 center = vec2(0.5, 0.5);
    vec2 texCoord = coord;
    float currentAngle = atan(texCoord.y - center.y, texCoord.x - center.x);

    if (currentAngle < 0.0) {
        currentAngle += 2.0 * PI;
    }

    // Count active textures
    int activeCount = tex1_active + tex2_active + tex3_active + tex4_active + tex5_active +
                      tex6_active + tex7_active + tex8_active + tex9_active + tex10_active;

    // Calculate number of sections based on active textures
    int sections = max(1, activeCount);  // Ensure at least one section
    float angleStep = 2.0 * PI / float(sections);
    int sectionIndex = int(currentAngle / angleStep);

    // Sample the texture corresponding to the current section
    vec4 finalColor = vec4(0.0);
    if (sectionIndex == 0 && tex1_active == 1) finalColor = texture(tex1, texCoord);
    else if (sectionIndex == 1 && tex2_active == 1) finalColor = texture(tex2, texCoord);
    else if (sectionIndex == 2 && tex3_active == 1) finalColor = texture(tex3, texCoord);
    else if (sectionIndex == 3 && tex4_active == 1) finalColor = texture(tex4, texCoord);
    else if (sectionIndex == 4 && tex5_active == 1) finalColor = texture(tex5, texCoord);
    else if (sectionIndex == 5 && tex6_active == 1) finalColor = texture(tex6, texCoord);
    else if (sectionIndex == 6 && tex7_active == 1) finalColor = texture(tex7, texCoord);
    else if (sectionIndex == 7 && tex8_active == 1) finalColor = texture(tex8, texCoord);
    else if (sectionIndex == 8 && tex9_active == 1) finalColor = texture(tex9, texCoord);
    else if (sectionIndex == 9 && tex10_active == 1) finalColor = texture(tex10, texCoord);

    outputColor = finalColor;
}
