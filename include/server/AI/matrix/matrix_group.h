#ifndef MATRIX_GROUP_H
#define MATRIX_GROUP_H

#include "utils/vector2.h"
#include "global_config.h"
#include <cmath>
#include <vector>
#include <algorithm>

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
    
    Vector2 final_target;    // El destino final de la misión
    Vector2 current_target;  // El objetivo dinámico actual (ej. siguiente paso en la carretera)
    
    int current_state;
    int current_action;
    float mission_timer;
    int target_count = 0; // Cuántos drones esperamos realmente

    bool isFull() const { return (int)children.size() >= target_count && target_count > 0; }

    MatrixGroup() : id(-1), rows(0), cols(0), center({0,0}), col_spacing(0), row_spacing(0), current_state(0), current_action(0), target_count(0) {}
    MatrixGroup(int id, Vector2 center, int cols, float col_spacing, int rows, float row_spacing)
        : id(id), rows(rows), cols(cols), center(center), col_spacing(col_spacing), row_spacing(row_spacing), current_state(0), current_action(0), target_count(rows*cols) 
    {
        // El ancho de la carretera determina la altura máxima permitida de la matriz
        int max_rows = std::max(1, static_cast<int>(HIGHWAY_WIDTH / row_spacing));
        
        if (this->rows > max_rows) {
            this->rows = max_rows;
            // Ensanchar la matriz para que entren todos los drones previstos
            this->cols = (target_count + this->rows - 1) / this->rows;
        }
    }
};

namespace MatrixAI {
    // Orquestador principal de la inteligencia de enjambre (Matrix AI)
    void orchestrate(MatrixGroup& self, const std::vector<MatrixGroup>& all_matrices, const std::vector<DroneChassis>& drones, float dt);
}

#endif
