#ifndef MISSION_ORCHESTRATOR_H
#define MISSION_ORCHESTRATOR_H

#include <vector>
#include "world/procedural_city.h"
#include "AI/matrix/matrix_group.h"
#include "missions/mission_calculator.h"

struct TransportMissionInfo {
    Vector2 startPos;
    Vector2 endPos;
    int pendingPackages;
    bool valid;
};

struct DeploymentPlan {
    int building_idx;
    int actual_batch;
    int drones_per_batch;
    int total_batches; 
    float xspacing;
    int total_drones;
    int drones_spawned;
    int mission_id; 
    int matrix_id;  // Nuevo: Para saber a qué matriz meter los drones
};

class MissionOrchestrator {
public:
    // Retorna los planes lógicos de despliegue
    // 1. Fase de Generación: Define la necesidad logística
    static TransportMissionInfo generateTransportMission(ProceduralCity& city);

    // 2. Fase de Planificación: Calcula la distribución física
    static std::vector<DeploymentPlan> createDeploymentPlans(
        const ProceduralCity& city, 
        Vector2 missionTarget, 
        int droneCount, 
        int buildingCount,
        int mission_id);

    // 3. Fase de Ejecución: Despliega y vincula las entidades
    static void startMission(class SwarmManager* swarm, class ProceduralCity* city);

    // Gestión de Estado Global
    static std::vector<DeploymentPlan>& getActiveSpawnPlans() { return active_spawn_plans; }
    static int getMissionCounter() { return mission_counter; }

private:
    static int mission_counter;
    static std::vector<DeploymentPlan> active_spawn_plans;
};

#endif
