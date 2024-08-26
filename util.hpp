#ifndef UTIL_HPP
#define UTIL_HPP

#include <GL/gl.h>

// see https://stackoverflow.com/questions/32432450
// thank you pmttavara!
struct defer_dummy {};
template <class F> struct deferrer { F f; ~deferrer() { f(); } };
template <class F> deferrer<F> operator*(defer_dummy, F f) { return {f}; }
#define DEFER_(LINE) zz_defer##LINE
#define DEFER(LINE) DEFER_(LINE)
#define defer auto DEFER(__LINE__) = defer_dummy{} *[&]()

extern GLuint create_shader(GLenum type, const GLchar *src, GLint len);

// it's the caller's responsibility to ensure that these string views are null
// terminated. (string views constructed from string literals are, but a
// substring view may not be, for example)
extern GLuint create_program(const std::string_view vtx_src, const std::string_view frag_src);

#endif
