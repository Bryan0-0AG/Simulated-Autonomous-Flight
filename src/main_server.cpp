#include "SimulationEngine.h"

int main() {
    // Iniciar en modo SERVIDOR (Calcula físicas, no dibuja, envía datos)
    SimulationEngine engine(EngineMode::SERVER);
    
    engine.init();
    engine.run();

    return 0;
}
