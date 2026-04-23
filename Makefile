CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude -Isrc

SRC = \
      src/main.cpp \
      src/physics/motion.cpp \
      src/physics/forces.cpp \
      src/utils/math_utils.cpp \
      src/world/world.cpp \
      src/rendering/BasicRenderer.cpp \
      src/rendering/camera.cpp \
      src/control/pid.cpp \
      src/control/controller.cpp \
      src/AI/decisions.cpp \
      src/AI/states.cpp \
      src/network/bridge.cpp
      
LIBS = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network
OUT = app

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LIBS)

clear:
	rm -f $(OUT)

run: all
	py ai_commander/brain.py