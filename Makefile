# ==========================================
# CONFIGURACIÓN DE COMPILADORES
# ==========================================
CXX      = g++
HIPCC    = hipcc
OUT      = app

# AMD ROCm Paths (Windows)
AMD_PATH = C:/Program Files/AMD/ROCm/7.1
AMD_INC  = -I"$(AMD_PATH)/include"
AMD_LIB  = -L"$(AMD_PATH)/lib" -lamdhip64

# Flags and Libraries
CXXFLAGS = -Wall -std=c++17 -Iinclude -Isrc $(AMD_INC)
HIPFLAGS = -g
LIBS     = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network $(AMD_LIB)

# ==========================================
# BLOCK 1: C++ / CPU (Source Files)
# ==========================================
SOURCES = src/main.cpp \
          src/swarm/swarm_manager.cpp \
          src/swarm/spawn_logic/matrix_group.cpp \
          src/swarm/spawn_logic/mission_orchestrator.cpp \
          src/SimulationEngine.cpp \
          src/utils/math_utils.cpp \
          src/world/world.cpp \
          src/rendering/renderer.cpp \
          src/rendering/camera.cpp \
          src/AI/drone_ai.cpp \
          src/AI/states.cpp \
          src/AI/matrix_ai.cpp \
          src/network/bridge.cpp \
          src/world/procedural_city.cpp \
          src/lab.cpp

# ==========================================
# BLOCK 2: AMD / GPU (Kernels)
# ==========================================
HIP_SRC      = src/swarm/swarm_dynamics.hip
HIP_OBJ      = build/swarm_dynamics.o

HIP_AI_SRC   = src/AI/matrix_ai.hip
HIP_AI_OBJ   = build/matrix_ai.o

# Dependencies
HEADERS = include/global_config.h \
          include/swarm/swarm_dynamics.h \
          include/swarm/swarm_manager.h \
          include/AI/states.h \
          include/AI/drone_ai.h \
          include/AI/matrix_ai.h

# ==========================================
# BUILD RULES
# ==========================================
all: build_dir $(HIP_OBJ) $(HIP_AI_OBJ)
	@echo "[BUILD] Compiling complete system with g++..."
	$(CXX) $(CXXFLAGS) $(SOURCES) $(HIP_OBJ) $(HIP_AI_OBJ) -o $(OUT) $(LIBS)

$(HIP_OBJ): $(HIP_SRC) $(HEADERS)
	@echo "[AMD] Compiling Drone Physics Kernel..."
	$(HIPCC) $(HIPFLAGS) -fno-exceptions -fno-rtti -c $(HIP_SRC) -o $(HIP_OBJ) -Iinclude

$(HIP_AI_OBJ): $(HIP_AI_SRC) $(HEADERS)
	@echo "[AMD] Compiling Matrix AI Kernel..."
	$(HIPCC) $(HIPFLAGS) -fno-exceptions -fno-rtti -c $(HIP_AI_SRC) -o $(HIP_AI_OBJ) -Iinclude

build_dir:
	@mkdir -p build

clean:
	@rm -rf build
	@rm -f $(OUT) $(OUT).exe

run: all
	py ai_commander/brain.py