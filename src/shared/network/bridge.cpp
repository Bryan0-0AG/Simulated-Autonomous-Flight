#include "network/bridge.h"
#include "global_config.h"
#include <iostream>
#include <sstream>
#include <cstring>

static bool sentBuildings = false;

NetworkBridge::NetworkBridge() : isOrchestratorConnected(false), isViewerConnected(false) {
    viewerSocket.setBlocking(false); // No bloquear el servidor mientras dibuja
}

NetworkBridge::~NetworkBridge() {
    disconnect();
}

bool NetworkBridge::connect(const std::string& ip, unsigned short port) {
    auto address = sf::IpAddress::resolve(ip);
    if (!address) return false;
    
    if (orchestratorSocket.connect(*address, port, sf::seconds(1)) != sf::Socket::Status::Done) {
        isOrchestratorConnected = false;
        return false;
    }
    orchestratorSocket.setBlocking(false); // <--- IMPORTANTE: No bloquear el simulador
    isOrchestratorConnected = true;
    return true;
}

void NetworkBridge::sendSwarmStatus(int total_drones, int critical_drones, int drones_in_mission, int live_missions, float avg_battery, float avg_speed, float avg_dist_to_target) {
    if (!isOrchestratorConnected) return;

    std::stringstream temp;
    temp << "{"
       << "\"total_drones\": " << total_drones << ", "
       << "\"critical_drones\": " << critical_drones << ", "
       << "\"drones_in_mission\": " << drones_in_mission << ", "
       << "\"live_missions\": " << live_missions << ", "
       << "\"avg_battery\": " << avg_battery << ", "
       << "\"avg_speed\": " << avg_speed << ", "
       << "\"avg_dist_to_target\": " << avg_dist_to_target
       << "}";

    std::string message = temp.str();
    if (orchestratorSocket.send(message.c_str(), message.size()) != sf::Socket::Status::Done) {
        isOrchestratorConnected = false;
    }
}

// =========================================================
// NUEVO: VISUALIZADOR REMOTO
// =========================================================

bool NetworkBridge::startVisualizerServer(unsigned short port) {
    if (viewerListener.listen(port) != sf::Socket::Status::Done) {
        std::cerr << "[BRIDGE] Error al iniciar el servidor de visualizacion en el puerto " << port << std::endl;
        return false;
    }
    viewerListener.setBlocking(false); // No detener la simulación esperando a que el cliente se conecte
    std::cout << "[BRIDGE] Visualization server listening on port " << port << std::endl;
    return true;
}

void NetworkBridge::sendPositionsToViewer(const std::vector<DroneChassis>& drones) {
    // 1. Aceptar conexion si alguien nuevo intenta conectarse
    if (!isViewerConnected) {
        if (viewerListener.accept(viewerSocket) == sf::Socket::Status::Done) {
            std::cout << "[BRIDGE] ¡Cliente visualizador conectado!" << std::endl;
            viewerSocket.setBlocking(false);
            isViewerConnected = true;
            sentBuildings = false; // Reset so buildings get sent to new client
        }
    }

    // 2. Si hay alguien conectado, enviarle las posiciones
    if (isViewerConnected && !drones.empty()) {
        // Primero enviamos la cantidad de drones
        uint32_t num_drones = drones.size();
        std::size_t sent_header = 0;
        if (viewerSocket.send(&num_drones, sizeof(num_drones), sent_header) == sf::Socket::Status::Disconnected) {
            isViewerConnected = false;
            return;
        }

        // Luego empaquetamos solo X e Y
        std::vector<float> positions;
        positions.reserve(drones.size() * 2);
        for (const auto& d : drones) {
            positions.push_back(d.position.x);
            positions.push_back(d.position.y);
        }

        // Enviamos el arreglo
        std::size_t sent = 0;
        if (viewerSocket.send(positions.data(), positions.size() * sizeof(float), sent) == sf::Socket::Status::Disconnected) {
            std::cout << "[BRIDGE] Cliente visualizador desconectado." << std::endl;
            isViewerConnected = false;
        }
    }
}

void NetworkBridge::sendBuildingsToViewer(const ProceduralCity& city) {
    if (!isViewerConnected || sentBuildings) return;

    const auto& buildings = city.getBuildings();
    // Marker: 0xFFFFFFFF signals "this is building data, not drones"
    uint32_t marker = 0xFFFFFFFF;
    std::size_t sent = 0;
    if (viewerSocket.send(&marker, sizeof(marker), sent) != sf::Socket::Status::Done) return;

    // Number of buildings
    uint32_t count = buildings.size();
    if (viewerSocket.send(&count, sizeof(count), sent) != sf::Socket::Status::Done) return;

    // Each building: x, y, w, h, type (5 floats)
    std::vector<float> data;
    data.reserve(count * 5);
    for (const auto& b : buildings) {
        data.push_back(b.bounds.position.x);
        data.push_back(b.bounds.position.y);
        data.push_back(b.bounds.size.x);
        data.push_back(b.bounds.size.y);
        data.push_back(static_cast<float>(static_cast<int>(b.type)));
    }

    (void)viewerSocket.send(data.data(), data.size() * sizeof(float), sent);
    sentBuildings = true;
    std::cout << "[BRIDGE] Sent " << count << " buildings to viewer." << std::endl;
}

bool NetworkBridge::connectToCloud(const std::string& ip, unsigned short port) {
    auto address = sf::IpAddress::resolve(ip);
    if (!address) return false;

    viewerSocket.setBlocking(true); // El cliente puede bloquearse esperando conectar
    if (viewerSocket.connect(*address, port, sf::seconds(5)) != sf::Socket::Status::Done) {
        std::cerr << "[BRIDGE] No se pudo conectar a la nube en " << ip << ":" << port << std::endl;
        isViewerConnected = false;
        return false;
    }
    std::cout << "[BRIDGE] Conectado exitosamente a la nube AMD." << std::endl;
    viewerSocket.setBlocking(false); // Para recibir datos fluido
    isViewerConnected = true;
    return true;
}

bool NetworkBridge::receivePositions(std::vector<DroneChassis>& drones) {
    if (!isViewerConnected) return false;

    // Usamos variables estáticas para acumular el estado del frame incompleto
    static uint32_t expected_drones = 0;
    static std::vector<float> receive_buffer;
    static std::size_t total_received_bytes = 0;

    // Fase 1: Leer el encabezado (cantidad de drones) si no estamos a mitad de un frame
    if (expected_drones == 0) {
        std::size_t received_header = 0;
        sf::Socket::Status status = viewerSocket.receive(&expected_drones, sizeof(expected_drones), received_header);
        
        if (status == sf::Socket::Status::Disconnected) {
            isViewerConnected = false;
            std::cout << "[BRIDGE] Desconectado de la nube." << std::endl;
            return false;
        }
        
        // Si no hay datos, salimos y esperamos al siguiente frame
        if (status != sf::Socket::Status::Done || received_header != sizeof(expected_drones)) {
            expected_drones = 0; 
            return false;
        }

        // Preparamos el buffer para el nuevo frame
        receive_buffer.resize(expected_drones * 2);
        total_received_bytes = 0;
    }

    // Fase 2: Leer el cuerpo (posiciones)
    if (expected_drones > 0) {
        std::size_t bytes_to_receive = (expected_drones * 2 * sizeof(float)) - total_received_bytes;
        std::size_t received = 0;
        
        // Recibimos directamente en la parte del buffer que falta
        char* dest = reinterpret_cast<char*>(receive_buffer.data()) + total_received_bytes;
        sf::Socket::Status status = viewerSocket.receive(dest, bytes_to_receive, received);

        if (status == sf::Socket::Status::Disconnected) {
            isViewerConnected = false;
            expected_drones = 0;
            return false;
        }

        total_received_bytes += received;

        // Si ya completamos el frame
        if (total_received_bytes == expected_drones * 2 * sizeof(float)) {
            // Actualizamos el tamaño del vector real de drones (así no dibujamos drones fantasmas)
            if (drones.size() != expected_drones) {
                drones.resize(expected_drones);
            }
            
            // Asignamos las coordenadas
            for (std::size_t i = 0; i < expected_drones; ++i) {
                drones[i].position.x = receive_buffer[i * 2];
                drones[i].position.y = receive_buffer[(i * 2) + 1];
            }
            
            // Reseteamos para el próximo frame
            expected_drones = 0;
            return true;
        }
    }
    
    return false;
}

void NetworkBridge::pollCommands() {
    if (!isOrchestratorConnected) {
        // Intentar reconectar automáticamente cada cierto tiempo
        static sf::Clock retryClock;
        if (retryClock.getElapsedTime().asSeconds() > 2.0f) {
            connect(SERVER_IP, SERVER_PORT);
            retryClock.restart();
        }
        return;
    }

    char data[128];
    std::size_t received;
    sf::Socket::Status status = orchestratorSocket.receive(data, sizeof(data) - 1, received);

    if (status == sf::Socket::Status::Done) {
        data[received] = '\0';
        std::string cmd(data);
        
        // Comando: SET_TIME_SCALE:0.5
        if (cmd.find("SET_TIME_SCALE:") == 0) {
            try {
                float newScale = std::stof(cmd.substr(15));
                TIME_SCALE = newScale;
                std::cout << "[BRIDGE] Time Scale updated to: " << TIME_SCALE << std::endl;
            } catch (...) {
                std::cerr << "[BRIDGE] Error al parsear TIME_SCALE" << std::endl;
            }
        }
    } else if (status == sf::Socket::Status::Disconnected) {
        isOrchestratorConnected = false;
    }
}

void NetworkBridge::disconnect() {
    orchestratorSocket.disconnect();
    viewerSocket.disconnect();
    isOrchestratorConnected = false;
    isViewerConnected = false;
}
