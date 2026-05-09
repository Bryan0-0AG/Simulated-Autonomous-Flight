#ifndef MATRIX_GROUP_H
#define MATRIX_GROUP_H

#include "utils/vector2.h"
#include <cmath>
#include <vector>

struct DroneChassis;

struct MatrixChild {
    int drone_id; // Guardamos el ID, no el puntero (para evitar crashes por realloc)
    int row;
    int col;
};

class MatrixGroup {
public:
    int id;
    int rows, cols;
    Vector2 center;
    float col_spacing;
    float row_spacing;
    std::vector<MatrixChild> children;
    
    Vector2 final_target;
    
    int current_state;
    int current_action;
    float mission_timer;
    int target_count = 0; // Cuántos drones esperamos realmente

    bool isFull() const { return (int)children.size() >= target_count && target_count > 0; }

    MatrixGroup() : id(-1), rows(0), cols(0), center({0,0}), col_spacing(0), row_spacing(0), current_state(0), current_action(0) {}
    MatrixGroup(int id, Vector2 center, int cols, float col_spacing, int rows, float row_spacing);

    Vector2 getSlotPosition(int row, int col) const;
    void insertChild(int droneId, int r, int c);
    float getGlobalError(const std::vector<DroneChassis>& drones) const;
    void moveTo(Vector2 newCenter);
    void updateMission(float dt);
    void reshape(int newCols, std::vector<DroneChassis>& drones);
};

#endif
