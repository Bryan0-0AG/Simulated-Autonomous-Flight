#include "AI/matrix/highways.h"
#include "global_config.h"
#include <cmath>

namespace MatrixAI {
namespace Highways {

    // Variables estáticas para asignación secuencial
    static int next_outbound = 0;
    static int next_inbound = 0;

    void assignLane(MatrixGroup& self) {
        bool goingRight = (self.final_target.x >= self.center.x);
        
        if (goingRight) {
            self.lane = next_outbound;
            next_outbound = (next_outbound + 1) % HIGHWAY_LANES_PER_DIR;
        } else {
            // Los carriles inbound se manejan a partir del offset HIGHWAY_LANES_PER_DIR
            self.lane = HIGHWAY_LANES_PER_DIR + next_inbound;
            next_inbound = (next_inbound + 1) % HIGHWAY_LANES_PER_DIR;
        }

        self.center.y = getAssignedAltitude(self);
        self.final_target.y = self.center.y; // El destino final Y se mantiene en su propio carril
    }

    float getAssignedAltitude(const MatrixGroup& self) {
        if (self.lane < HIGHWAY_LANES_PER_DIR) {
            // Es un carril Outbound
            return HIGHWAY_Y_OUTBOUND + (self.lane * HIGHWAY_LANE_SPACING);
        } else {
            // Es un carril Inbound
            int local_lane = self.lane - HIGHWAY_LANES_PER_DIR;
            return HIGHWAY_Y_INBOUND + (local_lane * HIGHWAY_LANE_SPACING);
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
