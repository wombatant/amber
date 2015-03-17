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
//#include <SDL2/SDL_opengl.h>

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
	auto vs = buildVertShader(vert, "vshad");
	auto fs = buildFragShader(frag, "fshad");
	auto prgm = glCreateProgram();
	glAttachShader(prgm, vs);
	glAttachShader(prgm, fs);
	glLinkProgram(prgm);
	return prgm;
}

GLuint buildRect(float vertices[], int size) {
	// vao
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	// vbo
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	// ebo
	GLuint ebo;
	glGenBuffers(1, &ebo);
	GLuint elms[] = {
		0, 1, 2,
		2, 3, 0
	};
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elms), elms, GL_STATIC_DRAW);
	return ebo;
}

}


// test code

int main(int argc, char *argv[]) {
	using namespace amber;
	SDL_Init(SDL_INIT_VIDEO);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	auto window = SDL_CreateWindow("Amber", 100, 100, 800, 600, SDL_WINDOW_OPENGL);
	auto context = SDL_GL_CreateContext(window);
	glewExperimental = GL_TRUE;
	glewInit();

	// set up scene
	float rect[] = {
		-0.5f, 0.5f, // top left
		 0.0f, 0.5f, // top right
		 0.0f, 0.0f, // bottom right
		-0.5f, 0.0f, // bottom left
	};
	buildRect(rect, sizeof(rect));
	auto shader = buildShaderProgram(vshad, fshad, 0);
	glUseProgram(shader);
	auto posAttrib = glGetAttribLocation(shader, "position");
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(posAttrib);

	// run
	for (bool running = true; running;) {
		// handle events
		SDL_Event sev;
		if (SDL_WaitEventTimeout(&sev, 3) != 0) {
			// got event
			const auto t = sev.type;
			if (t == SDL_QUIT) {
				SDL_GL_DeleteContext(context);
				SDL_Quit();
				running = false;
			}
		} else {
			// timeout
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			SDL_GL_SwapWindow(window);
			std::cout << "Narf!\n";
		}
	}

	return 0;
}
