#ifndef NETWORK_BRIDGE_H
#define NETWORK_BRIDGE_H

#include <vector>
#include <SFML/Network.hpp>
#include <string>
#include "drone_dynamics.h"
#include "world/procedural_city.h"

class NetworkBridge {
public:
    NetworkBridge();
    ~NetworkBridge();

    // --- LLM Orchestrator (Python) ---
    bool connect(const std::string& ip, unsigned short port);
    void sendSwarmStatus(
        int total_drones, 
        int critical_drones, 
        int drones_in_mission, 
        int live_missions,
        float avg_battery, 
        float avg_speed, 
        float avg_dist_to_target
    );
    void disconnect();
    void pollCommands(); // Leer comandos desde Python (ej: TIME_SCALE)

    // --- NUEVO: Visualizador Remoto (C++ a C++) ---
    // Para el Servidor (Nube AMD)
    bool startVisualizerServer(unsigned short port);
    void sendPositionsToViewer(const std::vector<DroneChassis>& drones);
    void sendBuildingsToViewer(const ProceduralCity& city);

    // Para el Cliente (Tu PC Local)
    bool connectToCloud(const std::string& ip, unsigned short port);
    bool receivePositions(std::vector<DroneChassis>& drones);

private:
    // Sockets para Python
    sf::TcpSocket orchestratorSocket;
    bool isOrchestratorConnected;

    // Sockets para el Visor Remoto
    sf::TcpListener viewerListener;
    sf::TcpSocket viewerSocket;
    bool isViewerConnected;
};

#endif
