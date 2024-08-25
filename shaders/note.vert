#version 460 core
layout(location = 0) in ivec2 pix;
layout(location = 0) uniform vec2 scrSize = vec2(400, 600);
layout(location = 1) uniform vec3 color = vec3(1.0, 0.0, 0.0);
out vec3 oColor;
void main() {
	oColor = color;
	vec2 norm = 2.0 * vec2(pix) / scrSize - 1.0;
	gl_Position = vec4(norm, 0.0, 1.0);
}
