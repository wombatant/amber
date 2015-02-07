#ifndef AMBER_HPP
#define AMBER_HPP

#include <GL/gl.h>


namespace amber {

extern const GLchar *vshad;
extern const GLchar *fshad;

GLuint buildVertShader(const GLchar *src, const char *shaderName = "unnamed vert shader");

GLuint buildFragShader(const GLchar *src, const char *shaderName = "unnamed frag shader");

GLuint buildVbo(float vertices[], int size);

}

#endif
