#include "AI/matrix/actions.h"
#include "AI/matrix/states.h"
#include "AI/matrix/properties.h"
#include "AI/drone_ai.h" // For drone target update

namespace MatrixAI {
namespace Actions {

    void executeMove(MatrixGroup& self, Vector2 new_target) {
        self.center = new_target;
        self.current_action = States::toInt(States::MatrixAction::FOLLOW_MATRIX);
    }

    void executeWait(MatrixGroup& self, float dt) {
        // Para esperar, le decimos que vaya a donde ya está
        self.center = self.center; // redundant, already handled
        self.current_action = States::toInt(States::MatrixAction::WAITING);
    }

    void reshape(MatrixGroup& self, int newCols, std::vector<DroneChassis>& drones) {
        if (newCols < 1 || self.children.empty()) return;

        self.cols = newCols;
        self.rows = (static_cast<int>(self.children.size()) + self.cols - 1) / self.cols;

        for (int i = 0; i < (int)self.children.size(); ++i) {
            int row_from_top = i / self.cols;
            int r = (self.rows - 1) - row_from_top;
            int c = i % self.cols;

            self.children[i].row = r;
            self.children[i].col = c;

            int droneId = self.children[i].drone_id;
            if (droneId >= 0 && droneId < (int)drones.size()) {
                drones[droneId].group_row = r;
                drones[droneId].group_col = c;
                drones[droneId].target = Properties::getSlotPosition(self, r, c);
            }
        }
    }

}
}
