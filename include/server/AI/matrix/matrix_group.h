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
    Vector2 velocity;        // Nueva variable para suavizar el movimiento y la repulsión
    float col_spacing;
    float row_spacing;
    std::vector<MatrixChild> children;
    
    Vector2 final_target;    // El destino final de la misión
    Vector2 current_target;  // El objetivo actual (waypoint de la carretera)
    Vector2 dynamic_target;  // El objetivo final con repulsión aplicada
    
    int current_state;
    int current_action;
    float mission_timer;     // Temporizador para estados de espera
    int mission_id;          // ID de la misión a la que pertenece esta matriz
    int target_count = 0; // Cuántos drones esperamos realmente
    int lane = 0;

    bool isFull() const { return (int)children.size() >= target_count && target_count > 0; }

    MatrixGroup() : id(-1), rows(0), cols(0), center({0,0}), velocity({0,0}), col_spacing(0), row_spacing(0), 
        final_target({0,0}), current_target({0,0}), dynamic_target({0,0}),
        current_state(0), current_action(0), mission_timer(0.0f), mission_id(0), target_count(0), lane(0) {}
    MatrixGroup(int id, Vector2 center, int cols, float col_spacing, int rows, float row_spacing)
        : id(id), rows(rows), cols(cols), center(center), velocity({0,0}), col_spacing(col_spacing), row_spacing(row_spacing), 
          final_target({0,0}), current_target(center), dynamic_target(center),
          current_state(0), current_action(0), mission_timer(0.0f), mission_id(0), target_count(rows*cols), lane(0) 
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
