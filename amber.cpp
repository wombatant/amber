#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <math.h>

#include <GLES3/gl3.h>

#include <SDL2/SDL.h>

#include "amber.hpp"


namespace amber {

using std::vector;

const GLchar *vshad = 
	"#version 150\n"
	"in vec2 position;"
	"in vec2 offset;"
	"void main() {"
	"    gl_Position = vec4(position, 0.0, 1.0) + vec4(offset, 0.0, 0.0);"
	"}";

const GLchar *fshad = 
	"#version 150\n"
	"out vec4 outColor;"
	"void main() {"
	"    outColor = vec4(0.0, 0.7, 1.0, 1.0);"
	"}";

GLuint buildShader(GLuint shaderType, const GLchar *src, const char *shaderName) {
	auto shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		static const auto errMsgSize = 1000;
		char errMsg[errMsgSize];
		glGetShaderInfoLog(shader, errMsgSize, nullptr, errMsg);
		fprintf(stderr, "shader compile error in %s:\n%s\n", shaderName, errMsg);
		glDeleteShader(shader);
		shader = 0;
	}
	return shader;
}

GLuint buildVertShader(const GLchar *src, const char *shaderName) {
	return buildShader(GL_VERTEX_SHADER, src, shaderName);
}

GLuint buildFragShader(const GLchar *src, const char *shaderName) {
	return buildShader(GL_FRAGMENT_SHADER, src, shaderName);
}

GLuint buildShaderProgram(const GLchar *vert, const GLchar *frag) {
	GLuint prgm = 0;
	const auto vs = buildVertShader(vert, "vshad");
	if (!vs) {
		glDeleteShader(vs);
	} else {
		const auto fs = buildFragShader(frag, "fshad");
		if (!fs) {
			// cleanup shaders that were created
			glDeleteShader(fs);
		} else {
			prgm = glCreateProgram();
			if (prgm) {
				glAttachShader(prgm, vs);
				glAttachShader(prgm, fs);
				glLinkProgram(prgm);
			}
		}
	}
	return prgm;
}


// BEGIN: Rect

Rect buildRect(float x, float y, float w, float h) {
	const float vertices[] = {
		    x,     y, // bottom left
		x + w,     y, // bottom right
		x + w, y + h, // top right
		    x, y + h, // top left
	};
	// vbo
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// ebo
	GLuint ebo;
	glGenBuffers(1, &ebo);
	const GLuint elms[] = {
		0, 1, 2,
		2, 3, 0
	};
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elms), elms, GL_STATIC_DRAW);
	// return Rect
	Rect rect;
	rect.vbo = vbo;
	rect.ebo = ebo;
	return rect;
}

void bind(Rect rect) {
	glBindBuffer(GL_ARRAY_BUFFER, rect.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rect.ebo);
}

void destroy(Rect rect) {
	glDeleteBuffers(1, &rect.ebo);
	glDeleteBuffers(1, &rect.vbo);
}

// END: Rect

}


// test code

int main(int argc, char *argv[]) {
	using namespace amber;
	SDL_Init(SDL_INIT_VIDEO);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	const auto dpiScale = 1;
	const auto window = SDL_CreateWindow("Amber", 100, 100, 800 * dpiScale, 600 * dpiScale, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	const auto context = SDL_GL_CreateContext(window);

	// set up scene

	// rect
	const auto rect = buildRect(0, 0);
	const auto shader = buildShaderProgram(vshad, fshad);

	// setup vaos
	vector<GLuint> vao(1);
	glGenVertexArrays(vao.size(), vao.data());

	// vao
	glBindVertexArray(vao[0]);
	bind(rect);
	glUseProgram(shader);
	const auto posAttrib = glGetAttribLocation(shader, "position");
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(posAttrib);

	// run
	for (auto running = true; running;) {
		// handle events
		SDL_Event sev;
		if (SDL_WaitEventTimeout(&sev, 3) != 0) {
			// got event
			const auto t = sev.type;
			switch (t) {
				case SDL_QUIT: {
					destroy(rect);
					SDL_GL_DeleteContext(context);
					SDL_Quit();
					running = false;
					break;
				}
				case SDL_KEYDOWN: {
					if (sev.key.keysym.scancode == SDL_SCANCODE_Q) {
						destroy(rect);
						SDL_GL_DeleteContext(context);
						SDL_Quit();
						running = false;
					}
					break;
				}
			}
		} else {
			// timeout
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			for (const GLuint v : vao) {
				glBindVertexArray(v);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			}
			SDL_GL_SwapWindow(window);
		}
	}

	destroy(rect);

	return 0;
}
