#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <math.h>

//#define GL3_PROTOTYPES 1
//#define GL_GLEXT_PROTOTYPES 1
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>

#include <SDL2/SDL.h>

#include "amber.hpp"


namespace amber {

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
	"    outColor = vec4(1.0, 1.0, 1.0, 1.0);"
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
		printf("shader compile error in %s:\n%s\n", shaderName, errMsg);
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

GLuint buildShaderProgram(const GLchar *vert, const GLchar *frag, GLuint buff) {
	GLuint vs, fs, prgm = 0;
	vs = buildVertShader(vert, "vshad");
	if (!vs) {
		glDeleteShader(vs);
	} else {
		fs = buildFragShader(frag, "fshad");
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
	float vertices[] = {
		    x,     y, // top left
		x + w,     y, // top right
		x + w, y + h, // bottom right
		    x, y + h, // bottom left
	};
	// vbo
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// ebo
	GLuint ebo;
	glGenBuffers(1, &ebo);
	GLuint elms[] = {
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

// END: Rect

}


// test code

int main(int argc, char *argv[]) {
	using namespace amber;
	SDL_Init(SDL_INIT_VIDEO);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	auto window = SDL_CreateWindow("Amber", 100, 100, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	auto context = SDL_GL_CreateContext(window);
	glewExperimental = GL_TRUE;
	glewInit();

	// set up scene

	// vao
	GLuint vao;
	glGenVertexArrays(2, &vao);
	glBindVertexArray(vao);

	// ebo
	auto rect = buildRect(0, 0);
	auto shader = buildShaderProgram(vshad, fshad, 0);
	glUseProgram(shader);
	auto posAttrib = glGetAttribLocation(shader, "position");
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(posAttrib);

	// vao2
	GLuint vao2;
	glGenVertexArrays(2, &vao2);
	glBindVertexArray(vao2);

	// ebo2
	auto rect2 = buildRect(-0.5f, -0.5f);
	auto shader2 = buildShaderProgram(vshad, fshad, 0);
	glUseProgram(shader2);
	auto posAttrib2 = glGetAttribLocation(shader2, "position");
	glVertexAttribPointer(posAttrib2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(posAttrib2);

	// run
	for (auto running = true; running;) {
		// handle events
		SDL_Event sev;
		if (SDL_WaitEventTimeout(&sev, 3) != 0) {
			//std::cout << "Event\n";
			// got event
			const auto t = sev.type;
			switch (t) {
				case SDL_QUIT: {
					SDL_GL_DeleteContext(context);
					SDL_Quit();
					running = false;
					break;
				}
				case SDL_KEYDOWN: {
					if (sev.key.keysym.scancode == SDL_SCANCODE_Q) {
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
			// rect 1
			glBindVertexArray(vao);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			// rect 2
			glBindVertexArray(vao2);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			SDL_GL_SwapWindow(window);
			//std::cout << "Timeout\n";
		}
	}

	return 0;
}
