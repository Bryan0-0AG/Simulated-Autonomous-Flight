CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude

SRC = src/main.cpp src/physics/motion.cpp
OUT = app

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)

run:
	./$(OUT)