#include "AI/matrix/matrix_group.h"
#include "AI/matrix/decisions.h"
#include "AI/matrix/properties.h"
#include "AI/matrix/states.h"

namespace MatrixAI {

    // Orquestador elemental del sistema Matrix AI
    void orchestrate(MatrixGroup& self, const std::vector<MatrixGroup>& all_matrices, const std::vector<DroneChassis>& drones, float dt) {
        if (self.children.empty()) return;
        
        // Delegamos todo el flujo elemental de la matriz al módulo de Decisiones.
        // Decisions internamente usará Properties, Perceptions, Highways y Actions.
        Decisions::evaluateAndDecide(self, all_matrices, drones, dt);
    }

}
