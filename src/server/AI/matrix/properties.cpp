#include "AI/matrix/properties.h"
#include "AI/matrix/states.h"
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
        
        // Add a safety margin equivalent to half spacing per side
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

        // Add safety margin (drone size or spacing)
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

    void applyDynamicRepulsion(MatrixGroup& self, const std::vector<MatrixGroup>& all_matrices, const std::vector<DroneChassis>& drones, float dt) {
        if (self.children.empty()) return;

        Vector2 myPos = getAveragePosition(self, drones);
        Vector2 repulsionOffset = {0.0f, 0.0f};
        
        float myHalfW = (self.cols * self.col_spacing) * 0.5f;
        float myHalfH = (self.rows * self.row_spacing) * 0.5f;
        float myRadius = std::max(myHalfW, myHalfH);

        float strength = 1200.0f; // Softer and more progressive force

        for (const auto& other : all_matrices) {
            if (other.id == self.id || other.children.empty()) continue;

            Vector2 otherPos = getAveragePosition(other, drones);
            float otherHalfW = (other.cols * other.col_spacing) * 0.5f;
            float otherHalfH = (other.rows * other.row_spacing) * 0.5f;
            float otherRadius = std::max(otherHalfW, otherHalfH);
            
            float minSafeDist = myRadius + otherRadius + 500.0f; // Detect from much further away
            
            float dx = myPos.x - otherPos.x;
            float dy = myPos.y - otherPos.y;
            float distSq = dx * dx + dy * dy;

            if (distSq < minSafeDist * minSafeDist && distSq > 1.0f) {
                float dist = std::sqrt(distSq);
                float normDist = (minSafeDist - dist) / minSafeDist;
                
                // Linear Repulsion: much more predictable and smooth than exponential
                float force = normDist * strength;
                repulsionOffset.x += (dx / dist) * force;
                repulsionOffset.y += (dy / dist) * force * 0.5f; // Greater smoothness on the vertical axis

                // Frenado predictivo suave
                self.velocity.x *= 0.99f;
                self.velocity.y *= 0.99f;
            }
        }

        Vector2 desired_target;
        desired_target.x = self.current_target.x + repulsionOffset.x;
        desired_target.y = self.current_target.y + repulsionOffset.y;

        // Dynamic target smoothing to avoid vibrations (soft Lerp)
        float lerpFactor = 5.0f * dt; 
        if (lerpFactor > 1.0f) lerpFactor = 1.0f;
        
        self.dynamic_target.x += (desired_target.x - self.dynamic_target.x) * lerpFactor;
        self.dynamic_target.y += (desired_target.y - self.dynamic_target.y) * lerpFactor;

        // Store for telemetry visualization
        self.last_repulsion = repulsionOffset;
    }

    void updateMatrixPhysics(MatrixGroup& self, float dt) {
        // 1. Attraction to dynamic_target (smooth steering)
        float dx = self.dynamic_target.x - self.center.x;
        float dy = self.dynamic_target.y - self.center.y;
        float dist = std::sqrt(dx*dx + dy*dy);
        
        if (dist > 1.0f) {
            float speed = 400.0f;
            // Reduce acceleration if very close to avoid "overshoot"
            float arrivalScaling = std::min(1.0f, dist / 200.0f);
            self.velocity.x += (dx / dist) * speed * arrivalScaling * dt;
            self.velocity.y += (dy / dist) * speed * arrivalScaling * dt;
        }

        // 2. Virtual elastic ground (Soft Spring)
        // Instead of teleporting, apply an upward force if below 150m
        if (self.current_action != States::toInt(States::MatrixAction::LANDING)) {
            float minHeight = 150.0f;
            if (self.center.y < minHeight) {
                float penetration = minHeight - self.center.y;
                float springK = 15.0f; // Elastic constant
                self.velocity.y += penetration * springK; // Upward thrust
                self.velocity.y *= 0.8f; // Rebound damping
            }
        }

        // 3. Dynamic friction (Damping)
        float friction = 0.92f; 
        self.velocity.x *= friction;
        self.velocity.y *= friction;

        // 4. Integration
        self.center.x += self.velocity.x * dt;
        self.center.y += self.velocity.y * dt;
    }

    void insertChild(MatrixGroup& matrix, int droneId, int r, int c) {
        matrix.children.push_back({droneId, r, c});
    }

}
}
