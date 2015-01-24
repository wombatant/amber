amber: amber.cpp
	c++ -std=c++11 amber.cpp -o amber -lSDL2 -lGL -lGLU #-framework OpenGL
run: amber
	./amber
