#ifndef NETWORK_BRIDGE_H
#define NETWORK_BRIDGE_H

#include <SFML/Network.hpp>
#include <string>
#include <vector>
#include "body.h"

class NetworkBridge {
public:
    NetworkBridge();
    ~NetworkBridge();

    bool connect(const std::string& ip, unsigned short port);
    
    void sendSwarmStatus(
        int total_drones, 
        int critical_drones, 
        int drones_in_mission, 
        float avg_battery, 
        float avg_speed, 
        float avg_dist_to_target
    );
    
    void disconnect();

private:
    sf::TcpSocket socket;
    bool isConnected;
};

#endif
