CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude -Isrc

SRC = src/main.cpp src/physics/motion.cpp src/utils/math_utils.cpp src/environment/world.cpp src/rendering/BasicRenderer.cpp
LIBS = -lsfml-graphics -lsfml-window -lsfml-system
OUT = app

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LIBS)

clean:
	rm -f $(OUT)

run:
	./$(OUT)