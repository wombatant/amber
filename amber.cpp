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
	"void main() {"
	"    gl_Position = vec4(position, 0.0, 1.0);"
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

Rect buildRect(GLuint shaderPrgm, float x, float y, float w, float h) {
	Rect rect;
	rect.width = w;
	rect.height = h;
	const float vertices[] = {
		    x,     y, // bottom left
		x + w,     y, // bottom right
		x + w, y + h, // top right
		    x, y + h, // top left
	};

	// vbo
	glGenBuffers(1, &rect.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, rect.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
	// ebo
	glGenBuffers(1, &rect.ebo);
	const GLuint elms[] = {
		0, 1, 2,
		2, 3, 0
	};
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rect.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elms), elms, GL_STATIC_DRAW);

	// vao
	glGenVertexArrays(1, &rect.vao);
	glBindVertexArray(rect.vao);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, rect.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rect.ebo);
	glUseProgram(shaderPrgm);

	return rect;
}

void destroy(Rect rect) {
	glDeleteBuffers(1, &rect.ebo);
	glDeleteBuffers(1, &rect.vbo);
}

void bind(Rect rect, float x, float y) {
	glBindVertexArray(rect.vao);

	// setup  vbo
	glBindBuffer(GL_ARRAY_BUFFER, rect.vbo);
	const float vertices[] = {
		             x,               y, // bottom left
		x + rect.width,               y, // bottom right
		x + rect.width, y + rect.height, // top right
		             x, y + rect.height, // top left
	};

	glBindBuffer(GL_ARRAY_BUFFER, rect.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
}

void bind(Rect rect) {
	glBindVertexArray(rect.vao);
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

	const auto dpiScale = 2;
	const auto window = SDL_CreateWindow("Amber", 100, 100, 800 * dpiScale, 600 * dpiScale, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	const auto context = SDL_GL_CreateContext(window);

	// set up scene

	// rect
	const auto shader = buildShaderProgram(vshad, fshad);
	const auto rect = buildRect(shader, 0, 0);

	// setup vaos
	vector<GLuint> vao(1);
	vao[0] = rect.vao;
	bind(rect, -0.5, -0.5);

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
