amber: amber.cpp amber.hpp
	clang++ -g -std=c++11 -Wall -Wsign-compare amber.cpp -o amber -lSDL2 -lGL # -fsanitize=address -fno-omit-frame-pointer #-framework OpenGL
run: amber
	./amber
clean:
	rm -f amber
