all:
	g++ -std=c++17 -I./include -L./lib -o main $(wildcard ./src/*.cpp) -lopengl32 -lglfw3dll -lglew32