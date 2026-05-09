#include "AI/matrix/perceptions.h"
#include "utils/math_utils.h"
#include "AI/matrix/properties.h"
#include <iostream>

namespace MatrixAI {
namespace Perceptions {

    // Helper: Colisión AABB vs AABB
    bool checkAABBIntersect(const Properties::Rect& a, const Properties::Rect& b) {
        return (a.min.x <= b.max.x && a.max.x >= b.min.x &&
                a.min.y <= b.max.y && a.max.y >= b.min.y);
    }

    bool Vision(const MatrixGroup& self, Direction dir, float distance, const std::vector<MatrixGroup>& all_matrices, const std::vector<DroneChassis>& drones) {
        Properties::Rect my_bounds = Properties::getDynamicBounds(self, drones);
        
        // Extender nuestra caja (AABB) en la dirección especificada simulando un Raycast ancho
        switch(dir) {
            case Direction::UP:         my_bounds.max.y += distance; break;
            case Direction::DOWN:       my_bounds.min.y -= distance; break;
            case Direction::RIGHT:      my_bounds.max.x += distance; break;
            case Direction::LEFT:       my_bounds.min.x -= distance; break;
            case Direction::UP_RIGHT:   my_bounds.max.y += distance; my_bounds.max.x += distance; break;
            case Direction::UP_LEFT:    my_bounds.max.y += distance; my_bounds.min.x -= distance; break;
            case Direction::DOWN_RIGHT: my_bounds.min.y -= distance; my_bounds.max.x += distance; break;
            case Direction::DOWN_LEFT:  my_bounds.min.y -= distance; my_bounds.min.x -= distance; break;
        }

        for (const auto& other : all_matrices) {
            if (other.id == self.id) continue;
            
            Properties::Rect other_bounds = Properties::getDynamicBounds(other, drones);
            
            // Si nuestro raycast rectangular intersecta a la otra matriz, la "vemos"
            if (checkAABBIntersect(my_bounds, other_bounds)) {
                return true;
            }
        }
        return false;
    }

    bool Radar(const MatrixGroup& self, float radiusX, float radiusY, const std::vector<MatrixGroup>& all_matrices, const std::vector<DroneChassis>& drones) {
        Vector2 my_avg = Properties::getAveragePosition(self, drones);

        for (const auto& other : all_matrices) {
            if (other.id == self.id) continue;
            
            Vector2 other_avg = Properties::getAveragePosition(other, drones);
            float dx = std::abs(my_avg.x - other_avg.x);
            float dy = std::abs(my_avg.y - other_avg.y);

            if (dx <= radiusX && dy <= radiusY) {
                return true;
            }
        }
        return false;
    }

    bool ShouldIWait(const MatrixGroup& self, const MatrixGroup& other) {
        // Regla 1: La que ya está en misión activa tiene prioridad sobre la que está en despegue (STAGING)
        if (self.current_state == 0 && other.current_state != 0) return true; // Yo espero (estoy en staging)
        if (self.current_state != 0 && other.current_state == 0) return false; // Yo sigo (estoy en misión)

        // Regla 2: Si ambas están en el mismo estado, la de ID menor tiene prioridad
        if (self.id > other.id) return true; // Yo espero
        
        return false; // Yo sigo
    }

}
}
