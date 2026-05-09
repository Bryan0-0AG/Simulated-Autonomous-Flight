#include "SimulationEngine.h"
#include <SFML/System/Err.hpp>
#include <iostream>

class NullBuffer : public std::streambuf {
public:
    int overflow(int c) { return c; }
};

int main() {
    // Silenciar los warnings nativos de SFML (como "Partial sends might not be handled properly")
    static NullBuffer null_buffer;
    static std::ostream null_stream(&null_buffer);
    sf::err().rdbuf(null_stream.rdbuf());

    // Iniciar en modo SERVIDOR (Calcula físicas, no dibuja, envía datos)
    SimulationEngine engine(EngineMode::SERVER);
    
    engine.init();
    engine.run();

    return 0;
}
