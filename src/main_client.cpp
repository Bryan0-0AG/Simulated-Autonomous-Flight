#include "SimulationEngine.h"

int main() {
    // Iniciar en modo CLIENTE (Recibe datos por red, dibuja, no calcula físicas)
    SimulationEngine engine(EngineMode::CLIENT);
    
    engine.init();
    engine.run();

    return 0;
}
