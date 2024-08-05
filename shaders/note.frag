#version 460 core
in vec3 oColor;
out vec4 frag_col;
void main() {
	frag_col = vec4(oColor, 1.0);
}
