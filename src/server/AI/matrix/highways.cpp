#include "AI/matrix/highways.h"
#include "global_config.h"
#include <cmath>

namespace MatrixAI {
namespace Highways {

    float getAssignedAltitude(const MatrixGroup& self) {
        // En nuestro mundo virtual, determinamos la dirección horizontal
        bool goingRight = (self.final_target.x >= self.center.x);
        
        if (goingRight) {
            return HIGHWAY_Y_OUTBOUND; // Carril de ida (Capa Media-Baja)
        } else {
            return HIGHWAY_Y_INBOUND; // Carril de vuelta (Capa Media-Alta)
        }
    }

    Vector2 calculateNextWaypoint(const MatrixGroup& self) {
        float highway_y = getAssignedAltitude(self);
        
        float dist_x = std::abs(self.final_target.x - self.center.x);
        float dist_y = std::abs(self.center.y - highway_y);

        // Tolerancias de llegada a los checkpoints imaginarios
        float tolerance_y = 20.0f;
        float tolerance_x = 20.0f;

        // FASE 1: ASCENDING (Subir verticalmente como un elevador hasta la carretera)
        // Si no estamos en la carretera y todavía estamos lejos horizontalmente del destino final
        if (dist_y > tolerance_y && dist_x > tolerance_x) {
            return {self.center.x, highway_y}; 
        }
        
        // FASE 2: CRUISING (Avanzar horizontalmente por la carretera)
        // Ya estamos en la carretera, ahora avanzamos hasta la coordenada X del objetivo
        if (dist_y <= tolerance_y && dist_x > tolerance_x) {
            return {self.final_target.x, highway_y}; 
        }

        // FASE 3: DESCENDING (Bajar verticalmente como elevador hasta el objetivo final)
        // Ya estamos en la coordenada X correcta, caemos hacia la meta
        return self.final_target;
    }

}
}
