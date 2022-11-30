#version 150

uniform sampler2DRect tex;
uniform float time;
uniform vec2 plasma;
uniform vec2 dimensions;

in vec2 coord;
out vec4 outputColor;

float field(in vec3 p) {
	float strength = 7. + .03 * log(1.e-6 + fract(sin(time) * 4373.11));
	float accum = 0.;
	float prev = 0.;
	float tw = 0.;
	for (int i = 0; i < 32; ++i) {
		float mag = dot(p, p);
		p = abs(p) / mag + vec3(-.5, -.4, -1.5);
		float w = exp(-float(i) / 7.);
		accum += w * exp(-strength * pow(abs(mag - prev), 2.3));
		tw += w;
		prev = mag;
	}
	return max(0., 5. * accum / tw - .7);
}

void main() {
	vec2 uv = 2. * coord.xy / dimensions.xy - 1.;
	vec2 uvs = uv * dimensions.xy / max(dimensions.x, dimensions.y);
	vec3 p = vec3(uvs / 4., 0) + vec3(1., -1.3, 0.);
	p += .2 * vec3(sin(time / 16.), sin(time / 12.),  sin(time / 128.));
	float t = field(p);
	float v = (1. - exp((abs(uv.x) - 1.) * 6.)) * (1. - exp((abs(uv.y) - 1.) * 6.));
	outputColor = mix(.4, 1., v) * vec4(1.8 * t * t * t, 1.4 * t * t, t, 1.0);
}
