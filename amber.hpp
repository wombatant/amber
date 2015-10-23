#ifndef AMBER_HPP
#define AMBER_HPP

#include <GLES3/gl3.h>


namespace amber {

extern const GLchar *vshad;
extern const GLchar *fshad;

// BEGIN: Rect

struct Rect {
	float width, height;
	GLuint vao;
	GLuint ebo;
	GLuint vbo;
};

Rect buildRect(GLuint shaderPrgm, float x = 0, float y = 0, float w = 0.5f, float h = 0.5f);

void bind(Rect rect);

void destroy(Rect rect);

// END: Rect


GLuint buildVertShader(const GLchar *src, const char *shaderName = "unnamed vert shader");

GLuint buildFragShader(const GLchar *src, const char *shaderName = "unnamed frag shader");

GLuint buildVbo(float vertices[], int size);

}

#endif
