amber: amber.cpp amber.hpp
	c++ -std=c++11 -Wall -Wsign-compare amber.cpp -o amber -lSDL2 -lGL -lGLEW #-framework OpenGL -g -fsanitize=address -fno-omit-frame-pointer
run: amber
	./amber
clean:
	rm amber
