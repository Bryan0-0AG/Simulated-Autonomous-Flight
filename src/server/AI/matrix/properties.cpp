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

    Vector2 getVectorError(const MatrixGroup& matrix, const std::vector<DroneChassis>& drones) {
        if (matrix.children.empty()) return {0.0f, 0.0f};
        float totalX = 0, totalY = 0;
        for (const auto& child : matrix.children) {
            if (child.drone_id < 0 || child.drone_id >= (int)drones.size()) continue;
            const auto& d = drones[child.drone_id];
            totalX += d.error.x;
            totalY += d.error.y;
        }
        float size = matrix.children.size();
        return {totalX / size, totalY / size};
    }

    Vector2 getAveragePosition(const MatrixGroup& matrix, const std::vector<DroneChassis>& drones) {
        if (matrix.children.empty()) return matrix.center;
        float totalX = 0;
        float totalY = 0;
        int count = 0;
        for (const auto& child : matrix.children) {
            if (child.drone_id < 0 || child.drone_id >= (int)drones.size()) continue;
            const auto& d = drones[child.drone_id];
            totalX += d.position.x;
            totalY += d.position.y;
            count++;
        }
        if (count == 0) return matrix.center;
        return {totalX / count, totalY / count};
    }

    Vector2 getAverageVelocity(const MatrixGroup& matrix, const std::vector<DroneChassis>& drones) {
        if (matrix.children.empty()) return {0.0f, 0.0f};
        float totalX = 0, totalY = 0;
        for (const auto& child : matrix.children) {
            if (child.drone_id < 0 || child.drone_id >= (int)drones.size()) continue;
            const auto& d = drones[child.drone_id];
            totalX += d.velocity.x;
            totalY += d.velocity.y;
        }
        float size = matrix.children.size();
        return {totalX / size, totalY / size};
    }

    float getAverageBattery(const MatrixGroup& matrix, const std::vector<DroneChassis>& drones) {
        if (matrix.children.empty()) return 0.0f;
        float totalBat = 0;
        for (const auto& child : matrix.children) {
            if (child.drone_id < 0 || child.drone_id >= (int)drones.size()) continue;
            totalBat += drones[child.drone_id].battery;
        }
        return totalBat / matrix.children.size();
    }

    Vector2 getAverageRepulsionForce(const MatrixGroup& matrix, const std::vector<DroneChassis>& drones) {
        if (matrix.children.empty()) return {0.0f, 0.0f};
        float totalX = 0, totalY = 0;
        for (const auto& child : matrix.children) {
            if (child.drone_id < 0 || child.drone_id >= (int)drones.size()) continue;
            const auto& d = drones[child.drone_id];
            totalX += d.f_separation.x;
            totalY += d.f_separation.y;
        }
        float size = matrix.children.size();
        return {totalX / size, totalY / size};
    }

    float getAverageThrust(const MatrixGroup& matrix, const std::vector<DroneChassis>& drones) {
        if (matrix.children.empty()) return 0.0f;
        float totalThrust = 0;
        for (const auto& child : matrix.children) {
            if (child.drone_id < 0 || child.drone_id >= (int)drones.size()) continue;
            totalThrust += drones[child.drone_id].control_output.thrust;
        }
        return totalThrust / matrix.children.size();
    }

    float getAverageAngle(const MatrixGroup& matrix, const std::vector<DroneChassis>& drones) {
        if (matrix.children.empty()) return 0.0f;
        float totalAngle = 0;
        for (const auto& child : matrix.children) {
            if (child.drone_id < 0 || child.drone_id >= (int)drones.size()) continue;
            totalAngle += drones[child.drone_id].control_output.angle;
        }
        return totalAngle / matrix.children.size();
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

    Rect getDynamicBounds(const MatrixGroup& matrix, const std::vector<DroneChassis>& drones) {
        if (matrix.children.empty()) return getBounds(matrix);
        
        float minX = 1e9f, minY = 1e9f;
        float maxX = -1e9f, maxY = -1e9f;
        int count = 0;

        for (const auto& child : matrix.children) {
            if (child.drone_id < 0 || child.drone_id >= (int)drones.size()) continue;
            const auto& d = drones[child.drone_id];
            
            if (d.position.x < minX) minX = d.position.x;
            if (d.position.y < minY) minY = d.position.y;
            if (d.position.x > maxX) maxX = d.position.x;
            if (d.position.y > maxY) maxY = d.position.y;
            count++;
        }

        if (count == 0) return getBounds(matrix);

        // Añadir margen de seguridad (drone size o spacing)
        float margin = matrix.col_spacing / 2.0f;
        return {
            {minX - margin, minY - margin},
            {maxX + margin, maxY + margin}
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
