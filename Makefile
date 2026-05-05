# ==========================================
# CONFIGURACIÓN DE COMPILADORES
# ==========================================
CXX      = g++
HIPCC    = hipcc
OUT      = app

# Rutas de AMD (ROCm 7.1)
AMD_PATH = C:/Program Files/AMD/ROCm/7.1
AMD_INC  = -I"$(AMD_PATH)/include"
AMD_LIB  = -L"$(AMD_PATH)/lib" -lamdhip64

# Flags y Librerías
CXXFLAGS = -Wall -std=c++17 -Iinclude -Isrc $(AMD_INC)
HIPFLAGS = -g
LIBS     = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network $(AMD_LIB)

# ==========================================
# BLOQUE 1: C++ / CPU (Archivos normales)
# ==========================================
#src/physics/motion.cpp \
#src/physics/forces.cpp \
#src/control/pid.cpp \
#src/control/controller.cpp \

SRC = src/main.cpp \
      src/utils/math_utils.cpp \
      src/world/world.cpp \
      src/rendering/BasicRenderer.cpp \
      src/rendering/camera.cpp \
      src/AI/decisions.cpp \
      src/AI/states.cpp \
      src/network/bridge.cpp \
      src/world/procedural_city.cpp

# ==========================================
# BLOQUE 2: AMD / GPU (Kernels)
# ==========================================
HIP_SRC = src/HPC/swarm_dynamics.hip
HIP_OBJ = build/swarm_dynamics.o

# ==========================================
# REGLAS DE COMPILACIÓN
# ==========================================
all: build_dir $(HIP_OBJ)
	@echo "[BUILD] Compilando sistema completo con g++..."
	$(CXX) $(CXXFLAGS) $(SRC) $(HIP_OBJ) -o $(OUT) $(LIBS)

$(HIP_OBJ): $(HIP_SRC)
	@echo "[AMD] Compilando Kernels de GPU..."
	$(HIPCC) $(HIPFLAGS) -fno-exceptions -fno-rtti -c $(HIP_SRC) -o $(HIP_OBJ) -Iinclude

build_dir:
	@mkdir -p build

clean:
	@rm -rf build
	@rm -f $(OUT) $(OUT).exe

run: all
	py ai_commander/brain.py