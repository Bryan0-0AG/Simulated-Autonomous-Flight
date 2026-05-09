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

    bool Vision(const MatrixGroup& self, Direction dir, float distance, const std::vector<MatrixGroup>& all_matrices) {
        Properties::Rect my_bounds = Properties::getBounds(self);
        
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
            
            Properties::Rect other_bounds = Properties::getBounds(other);
            
            // Si nuestro raycast rectangular intersecta a la otra matriz, la "vemos"
            if (checkAABBIntersect(my_bounds, other_bounds)) {
                return true;
            }
        }
        return false;
    }

    bool Radar(const MatrixGroup& self, float radius, const std::vector<MatrixGroup>& all_matrices) {
        for (const auto& other : all_matrices) {
            if (other.id == self.id) continue;
            
            float dist = distance(self.center, other.center);
            if (dist <= radius) {
                return true;
            }
        }
        return false;
    }

    bool Communication(const MatrixGroup& self, std::string& out_message) {
        // TODO: Escuchar paquetes TCP o variables compartidas
        return false;
    }

}
}
