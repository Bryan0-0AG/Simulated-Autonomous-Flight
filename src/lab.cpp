#include "lab.h"
#include "swarm/swarm_manager.h"
#include "world/procedural_city.h"
#include <iostream>

void triggerLabMission(SwarmManager* swarm, ProceduralCity* city) {
    if (!swarm || !city) return;
    
    std::cout << "[LAB] Iniciando coordinación de misión experimental..." << std::endl;
    
    TransportMissionInfo info = swarm->startRandomTransportMission(*city);
    
    if (info.valid) {
        std::cout << "[LAB] Misión generada. Pendientes: " << info.pendingPackages << " paquetes." << std::endl;
        
        // Spawnea los drones cerca de la posicion inicial
        info.startPos.y += 1000.0f;
        size_t initial_matrix_count = swarm->getMatrixGroups().size();
        
        MissionOrchestrator::initiateDeployment(*city, info.startPos, info.pendingPackages, 5, swarm->getMatrixGroups(), swarm->getActiveSpawnPlans());
        
        // Asigna el target (endPos) como final_target a todas las matrices que se acaban de crear
        info.endPos.y += 1000.0f;
        auto& matrices = swarm->getMatrixGroups();
        for (size_t i = initial_matrix_count; i < matrices.size(); ++i) {
            matrices[i].final_target = info.endPos;
        }
    }
    
    std::cout << "[LAB] Misión de laboratorio generada y desplegada." << std::endl;
}
