CXX = g++
CXXFLAGS = -Wall -std=c++17 -Iinclude -Isrc

SRC = \
      src/main.cpp \
      src/physics/motion.cpp \
      src/physics/forces.cpp \
      src/utils/math_utils.cpp \
      src/environment/world.cpp \
      src/rendering/BasicRenderer.cpp \
      src/control/pid.cpp \
      src/control/controller.cpp \
      src/AI/decisions.cpp \
      src/AI/states.cpp
      
LIBS = -lsfml-graphics -lsfml-window -lsfml-system
OUT = app

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LIBS)

clear:
	rm -f $(OUT)

run: all
	py telemetry/run_sim.py