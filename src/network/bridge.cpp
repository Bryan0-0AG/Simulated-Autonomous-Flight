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
    
    sf::Socket::Status status = socket.connect(*address, port, sf::seconds(5));
    if (status != sf::Socket::Status::Done) {
        std::cerr << "[BRIDGE ERROR] No se pudo conectar al Orquestador Python en " << ip << ":" << port << std::endl;
        isConnected = false;
        return false;
    }
    std::cout << "[BRIDGE] Conectado exitosamente al Orquestador Python." << std::endl;
    isConnected = true;
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
        std::cerr << "[BRIDGE ERROR] Error enviando datos a Python." << std::endl;
        isConnected = false;
    }
}

void NetworkBridge::disconnect() {
    socket.disconnect();
    isConnected = false;
}
