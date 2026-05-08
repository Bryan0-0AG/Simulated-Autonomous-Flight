#include "swarm/spawn_logic/matrix_group.h"
#include "swarm/swarm_dynamics.h"
#include "AI/states.h"
#include <iostream>
#include <cmath>

MatrixGroup::MatrixGroup(int id, Vector2 center, int cols, float col_spacing, int rows, float row_spacing) 
    : id(id), rows(rows), cols(cols), center(center), col_spacing(col_spacing), row_spacing(row_spacing) {
    current_state = toInt(DroneState::FLYING);
    current_action = toInt(DroneAction::TAKEOFF);
    mission_timer = 0.0f;
}

Vector2 MatrixGroup::getSlotPosition(int row, int col) const {
    float localX = (col - (cols - 1) / 2.0f) * col_spacing;
    float localY = (row - (rows - 1) / 2.0f) * row_spacing;
    return { center.x + localX, center.y + localY };
}

void MatrixGroup::insertChild(int droneId, int r, int c) {
    children.push_back({droneId, r, c});
}

void MatrixGroup::moveTo(Vector2 newCenter) {
    center = newCenter;
}

void MatrixGroup::updateMission(float dt) {
    // La lógica de misión ahora reside en update_matrix_ai (matrix_ai.cpp)
    mission_timer += dt;
}

float MatrixGroup::getGlobalError(const std::vector<DroneChassis>& drones) const {
    if (children.empty()) return 0.0f;
    float totalDist = 0;
    for (const auto& child : children) {
        // Buscamos al dron por su ID (suponiendo que el ID es el índice, o buscando si fuera necesario)
        // En nuestro caso, el ID se asigna secuencialmente: drone.id = drones.size()
        // así que drones[child.drone_id] es seguro siempre que el ID sea el índice.
        if (child.drone_id < 0 || child.drone_id >= (int)drones.size()) continue;

        const auto& d = drones[child.drone_id];
        Vector2 target = getSlotPosition(child.row, child.col);
        float dx = d.position.x - target.x;
        float dy = d.position.y - target.y;
        totalDist += std::sqrt(dx*dx + dy*dy);
    }
    return totalDist / children.size();
}

void MatrixGroup::reshape(int newCols, std::vector<DroneChassis>& drones) {
    if (newCols < 1 || children.empty()) return;

    this->cols = newCols;
    this->rows = (static_cast<int>(children.size()) + cols - 1) / cols;

    for (int i = 0; i < (int)children.size(); ++i) {
        int row_from_top = i / cols;
        int r = (rows - 1) - row_from_top;
        int c = i % cols;

        children[i].row = r;
        children[i].col = c;

        // Synchronize with the drone entity
        int droneId = children[i].drone_id;
        if (droneId >= 0 && droneId < (int)drones.size()) {
            drones[droneId].group_row = r;
            drones[droneId].group_col = c;
        }
    }
}

