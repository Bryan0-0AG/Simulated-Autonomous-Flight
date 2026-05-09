#include "AI/matrix/properties.h"
#include "drone_dynamics.h"
#include <cmath>
namespace MatrixAI {
namespace Properties {

    float getGlobalError(const MatrixGroup& matrix, const std::vector<DroneChassis>& drones) {
        if (matrix.children.empty()) return 0.0f;
        float totalDist = 0;
        for (const auto& child : matrix.children) {
            if (child.drone_id < 0 || child.drone_id >= (int)drones.size()) continue;

            const auto& d = drones[child.drone_id];
            Vector2 target = getSlotPosition(matrix, child.row, child.col);
            float dx = d.position.x - target.x;
            float dy = d.position.y - target.y;
            totalDist += std::sqrt(dx*dx + dy*dy);
        }
        return totalDist / matrix.children.size();
    }

    Rect getBounds(const MatrixGroup& matrix) {
        float half_w = (matrix.cols > 1 ? (matrix.cols - 1) * matrix.col_spacing : 0.0f) / 2.0f;
        float half_h = (matrix.rows > 1 ? (matrix.rows - 1) * matrix.row_spacing : 0.0f) / 2.0f;
        
        // Agregar un margen de seguridad equivalente a medio espaciado por lado
        half_w += matrix.col_spacing / 2.0f;
        half_h += matrix.row_spacing / 2.0f;

        return {
            {matrix.center.x - half_w, matrix.center.y - half_h},
            {matrix.center.x + half_w, matrix.center.y + half_h}
        };
    }

    Vector2 getSlotPosition(const MatrixGroup& matrix, int row, int col) {
        float x_offset = (col - (matrix.cols - 1) / 2.0f) * matrix.col_spacing;
        float y_offset = (row - (matrix.rows - 1) / 2.0f) * matrix.row_spacing;
        return {matrix.center.x + x_offset, matrix.center.y + y_offset};
    }

    void insertChild(MatrixGroup& matrix, int droneId, int r, int c) {
        matrix.children.push_back({droneId, r, c});
    }

}
}
