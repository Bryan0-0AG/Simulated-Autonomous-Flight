# ==========================================
# DETECCIÓN DE SISTEMA OPERATIVO Y RUTAS
# ==========================================
CXX      = g++
OUT_SERVER = app_server
OUT_CLIENT = app_client

# Flags and Libraries
CXXFLAGS_COMMON = -Wall -std=c++17 -Iinclude -Iinclude/shared -Iinclude/server -Iinclude/client -Iinclude/core -Iinclude/telemetry

ifeq ($(OS),Windows_NT)
    # --------- WINDOWS (Local) ---------
    HIPCC    = hipcc
    AMD_PATH = C:/Program Files/AMD/ROCm/7.1
    AMD_INC  = -I"$(AMD_PATH)/include"
    AMD_LIB  = -L"$(AMD_PATH)/lib" -lamdhip64
    CXXFLAGS = $(CXXFLAGS_COMMON) $(AMD_INC)
    HIPFLAGS = -g
    LIBS     = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network $(AMD_LIB)
    CLEAN_CMD = rm -rf build && rm -f $(OUT_SERVER) $(OUT_SERVER).exe $(OUT_CLIENT) $(OUT_CLIENT).exe
else
    # --------- LINUX (AMD Cloud) ---------
    HIPCC    = /opt/rocm/bin/hipcc
    AMD_INC  = -I/opt/rocm/include
    AMD_LIB  = -L/opt/rocm/lib -lamdhip64
    CXXFLAGS = $(CXXFLAGS_COMMON) -fPIC -I/usr/local/include $(AMD_INC)
    HIPFLAGS = -g -fPIC
    LIBS     = -L/usr/local/lib -Wl,-rpath=/usr/local/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network $(AMD_LIB)
    CLEAN_CMD = rm -rf build && rm -f $(OUT_SERVER) $(OUT_CLIENT)
endif

# ==========================================
# BLOCK 1: C++ / CPU (Core Sources)
# ==========================================
CORE_SOURCES = src/server/swarm/swarm_manager.cpp \
          src/server/AI/matrix/matrix_group.cpp \
          src/server/missions/mission_orchestrator.cpp \
          src/server/missions/mission_calculator.cpp \
          src/server/swarm/spawners/matrix_spawner.cpp \
          src/server/swarm/spawners/drone_spawner.cpp \
          src/core/SimulationEngine.cpp \
          src/shared/utils/math_utils.cpp \
          src/shared/world/world.cpp \
          src/client/rendering/renderer.cpp \
          src/client/rendering/camera.cpp \
          src/server/AI/drone_ai.cpp \
          src/server/AI/drone_states.cpp \
          src/server/AI/matrix/properties.cpp \
          src/server/AI/matrix/perceptions.cpp \
          src/server/AI/matrix/decisions.cpp \
          src/server/AI/matrix/actions.cpp \
          src/server/AI/matrix/states.cpp \
          src/server/AI/matrix/highways.cpp \
          src/shared/network/bridge.cpp \
          src/shared/world/procedural_city.cpp \
          src/server/lab.cpp

# ==========================================
# BLOCK 2: AMD / GPU (Kernels)
# ==========================================
HIP_SRC = src/server/drone_dynamics.hip
HIP_OBJ = build/drone_dynamics.o

# Dependencies (Using a general wildcard for simplicity now that it's spread out)
HEADERS = $(wildcard include/**/*.h)

# ==========================================
# BUILD RULES
# ==========================================
all: server client

server: build_dir $(HIP_OBJ)
	@echo "[BUILD] Compiling Server..."
	$(CXX) $(CXXFLAGS) src/core/main_server.cpp $(CORE_SOURCES) $(HIP_OBJ) -o $(OUT_SERVER) $(LIBS)

client: build_dir $(HIP_OBJ)
	@echo "[BUILD] Compiling Client..."
	$(CXX) $(CXXFLAGS) src/core/main_client.cpp $(CORE_SOURCES) $(HIP_OBJ) -o $(OUT_CLIENT) $(LIBS)

$(HIP_OBJ): $(HIP_SRC)
	@echo "[AMD] Compiling GPU Kernels..."
	$(HIPCC) $(HIPFLAGS) -fno-exceptions -fno-rtti -c $(HIP_SRC) -o $(HIP_OBJ) $(CXXFLAGS_COMMON)


build_dir:
	@mkdir -p build

clean:
	@$(CLEAN_CMD)

run_server: server
	./$(OUT_SERVER)

run_client: client
	./$(OUT_CLIENT)

run_brain:
	py ai_commander/brain.py