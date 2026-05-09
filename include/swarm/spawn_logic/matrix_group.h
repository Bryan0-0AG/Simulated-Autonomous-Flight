#ifndef MATRIX_GROUP_H
#define MATRIX_GROUP_H

#include "../../utils/vector2.h"
#include <cmath>
#include <vector>

struct DroneChassis;

struct MatrixChild {
    int drone_id; // Guardamos el ID, no el puntero (para evitar crashes por realloc)
    int row;
    int col;
};

// Axis-Aligned Bounding Box for inter-matrix collision detection
struct MatrixBounds {
    float minX, maxX;
    float minY, maxY;
    // Check if this bounding box overlaps with another
    bool overlaps(const MatrixBounds& other) const {
        return minX < other.maxX && maxX > other.minX &&
               minY < other.maxY && maxY > other.minY;
    }
};

class MatrixGroup {
public:
    int id;
    int rows, cols;
    Vector2 center;
    float col_spacing;
    float row_spacing;
    std::vector<MatrixChild> children;
    
    Vector2 final_target;   // Ultimate mission destination
    Vector2 nav_target;     // Current navigation target (moves toward final_target gradually)
    
    int current_state;
    int current_action;
    float mission_timer;
    int target_count = 0;   // How many drones we expect in this matrix
    int assigned_lane = 0;  // Aerial highway lane index (set at creation)
    bool is_braking = false; // Set by forward vision when obstacle detected ahead
    float lane_change_cooldown = 0.0f; // Seconds remaining before next lane change is allowed
    bool is_changing_lane = false; // True while transitioning Y to new lane (X frozen)
    int previous_lane = -1;  // Last lane before current, prevents ping-pong return

    bool isFull() const { return (int)children.size() >= target_count && target_count > 0; }

    MatrixGroup() : id(-1), rows(0), cols(0), center({0,0}), col_spacing(0), row_spacing(0),
                    current_state(0), current_action(0), is_braking(false),
                    lane_change_cooldown(0.0f), is_changing_lane(false), previous_lane(-1) {}
    MatrixGroup(int id, Vector2 center, int cols, float col_spacing, int rows, float row_spacing);

    Vector2 getSlotPosition(int row, int col) const;
    void insertChild(int droneId, int r, int c);
    float getGlobalError(const std::vector<DroneChassis>& drones) const;
    void moveTo(Vector2 newCenter);
    void updateMission(float dt);
    void reshape(int newCols, std::vector<DroneChassis>& drones);
    MatrixBounds getBounds(float safetyMargin = 0.0f) const;
    float getLaneAltitude() const;
};

#endif
