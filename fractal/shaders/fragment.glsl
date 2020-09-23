#version 330 core

in vec4 pos;
out vec4 out_color;

uniform float zoom;
uniform int max_iterations;
uniform vec2 center;
uniform vec2 shift;
uniform float max_radius;
uniform sampler1D tex;

const vec4 fcolor = vec4(1, 0, 0, 1);
const vec4 scolor = vec4(0, 0, 1, 1);

void main() {
	vec2 nm = (pos.xy + center) * vec2(zoom) + shift;
	// vec2 nm = vec2(pos.x * (1.0 / zoom) + center.x, pos.y * (1.0 / zoom) + center.y);
	vec2 c = nm;
	float value = 0.0;
	int i = 0;
	for (; i < max_iterations && value < max_radius; ++i) {
		nm = vec2(nm.x * nm.x - nm.y * nm.y, nm.x * nm.y + nm.y * nm.x) + c;
		value = nm.x * nm.x + nm.y * nm.y;
	}
	if (value < max_radius) {
		// out_color = mix(fcolor, scolor, distance(nm, c) * 1.5);
		out_color = texture(tex, value);
	}
	else {
		out_color = vec4(0, 0, 0, 1.0);
	}
}
