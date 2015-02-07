amber: amber.cpp
	c++ -std=c++11 -Wall -Wsign-compare amber.cpp -o amber -lSDL2 -lGL -lGLU -lGLEW #-framework OpenGL
run: amber
	./amber
clean:
	rm amber
