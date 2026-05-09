#include "network/bridge.h"
#include <iostream>
#include <sstream>

NetworkBridge::NetworkBridge() : isConnected(false) {}

NetworkBridge::~NetworkBridge() {
    disconnect();
}

bool NetworkBridge::connect(const std::string& ip, unsigned short port) {
    auto address = sf::IpAddress::resolve(ip);
    if (!address) {
        std::cerr << "[BRIDGE ERROR] No se pudo resolver la IP: " << ip << std::endl;
        return false;
    }
    
    sf::Socket::Status status = socket.connect(*address, port, sf::seconds(1));
    if (status != sf::Socket::Status::Done) {
        std::cerr << "[BRIDGE ERROR] No se pudo conectar al Orquestador Python en " << ip << ":" << port << std::endl;
        isConnected = false;
        return false;
    }
    std::cout << "[BRIDGE] Conectado exitosamente al Orquestador Python." << std::endl;
    isConnected = true;
    socket.setBlocking(false); // Make socket non-blocking
    return true;
}

void NetworkBridge::sendSwarmStatus(
    int total_drones, 
    int critical_drones, 
    int drones_in_mission, 
    float avg_battery, 
    float avg_speed, 
    float avg_dist_to_target)
{
    if (!isConnected) return;

    // Construimos un JSON detallado manualmente
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
    
    if (socket.send(message.c_str(), message.size()) != sf::Socket::Status::Done) {
        // Non-blocking socket: send may not complete immediately, safe to ignore
    }
}

void NetworkBridge::sendMatricesStatus(const std::vector<MatrixGroup>& matrices) {
    if (!isConnected) return;

    std::stringstream temp;
    temp << "{\"matrices\":[";
    for (size_t i = 0; i < matrices.size(); ++i) {
        const auto& m = matrices[i];
        temp << "{\"id\":" << m.id 
             << ",\"p\":[" << m.center.x << "," << m.center.y << "]"
             << ",\"t\":[" << m.final_target.x << "," << m.final_target.y << "]"
             << ",\"s\":" << m.current_action
             << ",\"d\":" << m.children.size()
             << ",\"lane\":" << m.assigned_lane << "}";
        if (i < matrices.size() - 1) temp << ",";
    }
    temp << "]}";

    std::string message = temp.str();
    std::cout << "[BRIDGE] Enviando a Python: " << message << std::endl;
    if (socket.send(message.c_str(), message.size()) != sf::Socket::Status::Done) {
        // Ignoramos el error silenciosamente, pero evitamos el warning de compilación
    }
}

std::string NetworkBridge::receiveCommands() {
    if (!isConnected) return "";
    char buffer[4096];
    std::size_t received;
    sf::Socket::Status status = socket.receive(buffer, sizeof(buffer), received);
    if (status == sf::Socket::Status::Done) {
        return std::string(buffer, received);
    }
    return "";
}

void NetworkBridge::disconnect() {
    socket.disconnect();
    isConnected = false;
}
