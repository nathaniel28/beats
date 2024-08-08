#version 460 core
out vec4 fragColor;
in vec2 texCoords;
uniform sampler2D tex;
layout(location = 0) uniform float nCols;
layout(location = 1) uniform uint now;
layout(location = 2) uniform uint times[8];
void main() {
	uint t = now - times[int(texCoords.x * nCols)];
	vec3 col = texture(tex, texCoords).rgb;
	const uint flashDuration = 250;
	if (t < flashDuration) {
		vec3 flash;
		if (texCoords.y <= 0.2) {
			flash = mix(col, vec3(0.486, 0.729, 0.815), 2.0 * (0.2 - texCoords.y));
		} else {
			float x = 1.25 - 1.25 * texCoords.y;
			flash = mix(col, vec3(1.0), (1.0 - x * x) * 0.15);
		}
		col = mix(col, flash, 1.0 - float(t) / float(flashDuration));
	}
	fragColor = vec4(col, 1.0);
}
