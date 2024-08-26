#include <iostream>

#include <GL/glew.h>
#include <GL/gl.h>

#include "util.hpp"

void log_shader_err(GLuint shader) {
	GLchar log[256];
	GLsizei log_len;
	glGetShaderInfoLog(shader, sizeof(log), &log_len, log);
	std::cout.write(log, log_len) << '\n';
	if (log_len == sizeof(log))
		std::cout << "\n(log truncated)\n";
}

GLuint create_shader(GLenum type, const GLchar *src, GLint len) {
	GLuint shader = glCreateShader(type);
	if (!shader)
		return 0;
	const char *const srcs[] = { src };
	glShaderSource(shader, 1, srcs, &len);
	glCompileShader(shader);
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		std::cout.write(src, len) << '\n';
		log_shader_err(shader);
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

GLuint create_program(const std::string_view vtx_src, const std::string_view frag_src) {
	GLuint vtx_shader = create_shader(GL_VERTEX_SHADER, vtx_src.data(), vtx_src.size());
	if (!vtx_shader)
		return 0;
	defer { glDeleteShader(vtx_shader); };
	GLuint frag_shader = create_shader(GL_FRAGMENT_SHADER, frag_src.data(), frag_src.size());
	if (!frag_shader)
		return 0;
	defer { glDeleteShader(frag_shader); };
	GLuint prog = glCreateProgram();
	if (!prog)
		return 0;
	glAttachShader(prog, vtx_shader);
	glAttachShader(prog, frag_shader);
	glLinkProgram(prog);
	GLint status;
	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		log_shader_err(prog);
		glDeleteProgram(prog); // pretty sure this doesn't call glDeleteShader on the shaders
		return 0;
	}
	return prog;
}
