# ==========================================
# DETECCIÓN DE SISTEMA OPERATIVO Y RUTAS
# ==========================================
CXX      = g++
OUT_SERVER = app_server
OUT_CLIENT = app_client

ifeq ($(OS),Windows_NT)
    # --------- WINDOWS (Local) ---------
    HIPCC    = hipcc
    AMD_PATH = C:/Program Files/AMD/ROCm/7.1
    AMD_INC  = -I"$(AMD_PATH)/include"
    AMD_LIB  = -L"$(AMD_PATH)/lib" -lamdhip64
    CXXFLAGS = -Wall -std=c++17 -Iinclude -Isrc $(AMD_INC)
    HIPFLAGS = -g
    LIBS     = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network $(AMD_LIB)
    CLEAN_CMD = rm -rf build && rm -f $(OUT_SERVER) $(OUT_SERVER).exe $(OUT_CLIENT) $(OUT_CLIENT).exe
else
    # --------- LINUX (AMD Cloud) ---------
    HIPCC    = /opt/rocm/bin/hipcc
    AMD_INC  = -I/opt/rocm/include
    AMD_LIB  = -L/opt/rocm/lib -lamdhip64
    CXXFLAGS = -Wall -std=c++17 -fPIC -Iinclude -Isrc -I/usr/local/include $(AMD_INC)
    HIPFLAGS = -g -fPIC
    LIBS     = -L/usr/local/lib -Wl,-rpath=/usr/local/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network $(AMD_LIB)
    CLEAN_CMD = rm -rf build && rm -f $(OUT_SERVER) $(OUT_CLIENT)
endif

# ==========================================
# BLOCK 1: C++ / CPU (Core Sources)
# ==========================================
CORE_SOURCES = src/swarm/swarm_manager.cpp \
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
HIP_SRC = src/swarm/swarm_dynamics.hip
HIP_OBJ = build/swarm_dynamics.o

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
all: server client

server: build_dir $(HIP_OBJ)
	@echo "[BUILD] Compiling Server..."
	$(CXX) $(CXXFLAGS) src/main_server.cpp $(CORE_SOURCES) $(HIP_OBJ) -o $(OUT_SERVER) $(LIBS)

client: build_dir $(HIP_OBJ)
	@echo "[BUILD] Compiling Client..."
	$(CXX) $(CXXFLAGS) src/main_client.cpp $(CORE_SOURCES) $(HIP_OBJ) -o $(OUT_CLIENT) $(LIBS)

$(HIP_OBJ): $(HIP_SRC) $(HEADERS)
	@echo "[AMD] Compiling GPU Kernels..."
	$(HIPCC) $(HIPFLAGS) -fno-exceptions -fno-rtti -c $(HIP_SRC) -o $(HIP_OBJ) -Iinclude

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