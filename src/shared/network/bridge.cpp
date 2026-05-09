#include "network/bridge.h"
#include <iostream>
#include <sstream>

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
    isOrchestratorConnected = true;
    return true;
}

void NetworkBridge::sendSwarmStatus(int total_drones, int critical_drones, int drones_in_mission, float avg_battery, float avg_speed, float avg_dist_to_target) {
    if (!isOrchestratorConnected) return;

    std::stringstream temp;
    temp << "{"
       << "\"total_drones\": " << total_drones << ", "
       << "\"critical_drones\": " << critical_drones << ", "
       << "\"drones_in_mission\": " << drones_in_mission << ", "
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
    std::cout << "[BRIDGE] Servidor de visualizacion escuchando en el puerto " << port << std::endl;
    return true;
}

void NetworkBridge::sendPositionsToViewer(const std::vector<DroneChassis>& drones) {
    // 1. Aceptar conexion si alguien nuevo intenta conectarse
    if (!isViewerConnected) {
        if (viewerListener.accept(viewerSocket) == sf::Socket::Status::Done) {
            std::cout << "[BRIDGE] ¡Cliente visualizador conectado!" << std::endl;
            viewerSocket.setBlocking(false);
            isViewerConnected = true;
        }
    }

    // 2. Si hay alguien conectado, enviarle las posiciones
    if (isViewerConnected) {
        // Empaquetamos solo X e Y para ahorrar ancho de banda
        std::vector<float> positions;
        positions.reserve(drones.size() * 2);
        for (const auto& d : drones) {
            positions.push_back(d.position.x);
            positions.push_back(d.position.y);
        }

        // Enviamos el arreglo en bytes. Pasamos 'sent' para suprimir la advertencia de envío parcial de SFML.
        std::size_t sent = 0;
        if (viewerSocket.send(positions.data(), positions.size() * sizeof(float), sent) == sf::Socket::Status::Disconnected) {
            std::cout << "[BRIDGE] Cliente visualizador desconectado." << std::endl;
            isViewerConnected = false;
        }
    }
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

    // Si el cliente acaba de arrancar, su vector de drones estara vacio
    if (drones.empty()) {
        drones.resize(10000); // 10k MAX
    }

    std::vector<float> buffer(drones.size() * 2);
    std::size_t received;
    
    sf::Socket::Status status = viewerSocket.receive(buffer.data(), buffer.size() * sizeof(float), received);
    
    if (status == sf::Socket::Status::Done && received > 0) {
        std::size_t count = received / (2 * sizeof(float));
        for (std::size_t i = 0; i < count && i < drones.size(); ++i) {
            drones[i].position.x = buffer[i * 2];
            drones[i].position.y = buffer[(i * 2) + 1];
        }
        return true;
    } else if (status == sf::Socket::Status::Disconnected) {
        isViewerConnected = false;
        std::cout << "[BRIDGE] Desconectado de la nube." << std::endl;
    }
    return false;
}

void NetworkBridge::disconnect() {
    orchestratorSocket.disconnect();
    viewerSocket.disconnect();
    isOrchestratorConnected = false;
    isViewerConnected = false;
}
