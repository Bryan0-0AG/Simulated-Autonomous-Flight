#ifndef NETWORK_BRIDGE_H
#define NETWORK_BRIDGE_H

#include <SFML/Network.hpp>
#include <string>
#include <vector>
#include "swarm/swarm_dynamics.h"
#include "swarm/spawn_logic/matrix_group.h"

class NetworkBridge {
public:
    NetworkBridge();
    ~NetworkBridge();

    bool connect(const std::string& ip, unsigned short port);
    bool getIsConnected() const { return isConnected; }
    
    void sendSwarmStatus(
        int total_drones, 
        int critical_drones, 
        int drones_in_mission, 
        float avg_battery, 
        float avg_speed, 
        float avg_dist_to_target
    );
    
    void sendMatricesStatus(const std::vector<MatrixGroup>& matrices);
    std::string receiveCommands();
    
    void disconnect();

private:
    sf::TcpSocket socket;
    bool isConnected;
};

#endif
