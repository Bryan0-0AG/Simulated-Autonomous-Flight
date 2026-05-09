#include "AI/matrix_ai.h"
#include "AI/states.h"
#include "global_config.h"
#include "swarm/swarm_dynamics.h" // GPUMatrix, launch_matrix_ai_kernel
#include <vector>
#include <iostream>
#include <cmath>

// ============================================================
// CPU-SIDE LANE CHANGE DECISION
// Excludes previous_lane to prevent ping-pong oscillation.
// ============================================================
static int findSafeLane(const MatrixGroup& me,
                        const std::vector<MatrixGroup>& all_matrices) {
    // Prefer going UP (away from buildings) over DOWN
    int candidates[] = { me.assigned_lane + 1, me.assigned_lane - 1 };

    for (int candidate : candidates) {
        if (candidate < 0 || candidate >= MAX_AERIAL_LANES) continue;
        // Anti-ping-pong: never go back to the lane you just left
        if (candidate == me.previous_lane) continue;
        // Safety: never go below lane 0 (closest to buildings)
        if (candidate < me.assigned_lane && me.assigned_lane <= 1) continue;

        bool laneFree = true;
        for (const auto& other : all_matrices) {
            if (other.id == me.id) continue;
            if (other.assigned_lane != candidate) continue;

            float dx = std::abs(other.center.x - me.center.x);
            if (dx < FORWARD_VISION_DISTANCE * 1.5f) {
                laneFree = false;
                break;
            }
        }
        if (laneFree) return candidate;
    }
    return -1; // All adjacent lanes blocked
}

// ============================================================
// TAKEOFF CLEARANCE CHECK
// Returns true if the assigned lane is clear for this matrix
// to transition into FOLLOW_MATRIX. If blocked, tries to ask
// the blocking matrix to move up one lane.
// ============================================================
static bool checkTakeoffClearance(MatrixGroup& me,
                                  std::vector<MatrixGroup>& all_matrices) {
    for (auto& other : all_matrices) {
        if (other.id == me.id) continue;
        if (other.assigned_lane != me.assigned_lane) continue;
        if (other.current_action != toInt(DroneAction::FOLLOW_MATRIX)) continue;

        // Check if this other matrix is close in X (would collide on entry)
        float dx = std::abs(other.center.x - me.center.x);
        float minSafe = (me.cols * me.col_spacing) / 2.0f
                      + (other.cols * other.col_spacing) / 2.0f
                      + MATRIX_SAFETY_MARGIN * 4.0f;

        if (dx < minSafe) {
            // Lane blocked! Try to ask the blocker to move up
            int blockerNewLane = other.assigned_lane + 1;
            if (blockerNewLane < MAX_AERIAL_LANES && other.lane_change_cooldown <= 0.0f) {
                // Check if the lane above is free for the blocker
                bool upperFree = true;
                for (const auto& check : all_matrices) {
                    if (check.id == other.id) continue;
                    if (check.assigned_lane != blockerNewLane) continue;
                    float checkDx = std::abs(check.center.x - other.center.x);
                    if (checkDx < FORWARD_VISION_DISTANCE) {
                        upperFree = false;
                        break;
                    }
                }
                if (upperFree) {
                    std::cout << "[CLEARANCE] Matrix " << me.id
                              << " asks Matrix " << other.id
                              << " to vacate lane " << other.assigned_lane
                              << " -> " << blockerNewLane << std::endl;
                    other.previous_lane = other.assigned_lane;
                    other.assigned_lane = blockerNewLane;
                    other.nav_target.y  = other.getLaneAltitude();
                    other.is_changing_lane = true;
                    other.lane_change_cooldown = 5.0f;
                    return true; // Lane will clear soon
                }
            }
            return false; // Can't clear, wait
        }
    }
    return true; // Lane is clear
}

// ============================================================
// MAIN ADAPTER: CPU packs data -> GPU computes -> CPU decides
// ============================================================
void update_matrix_ai(std::vector<MatrixGroup>& all_matrices,
                      std::vector<DroneChassis>& drones,
                      float dt) {
    int n = (int)all_matrices.size();
    if (n == 0) return;

    // 1. Pack MatrixGroup -> GPUMatrix
    std::vector<GPUMatrix> gpu_mats(n);
    for (int i = 0; i < n; i++) {
        const MatrixGroup& m = all_matrices[i];
        GPUMatrix& g = gpu_mats[i];

        g.id               = m.id;
        g.center_x         = m.center.x;
        g.center_y         = m.center.y;
        g.nav_target_x     = m.nav_target.x;
        g.nav_target_y     = m.nav_target.y;
        g.final_target_x   = m.final_target.x;
        g.final_target_y   = m.final_target.y;
        g.col_spacing      = m.col_spacing;
        g.row_spacing      = m.row_spacing;
        g.cols             = m.cols;
        g.rows             = m.rows;
        g.child_count      = (int)m.children.size();
        g.current_action   = m.current_action;
        g.assigned_lane    = m.assigned_lane;
        g.lane_change_cooldown = m.lane_change_cooldown;
        g.is_braking       = m.is_braking ? 1 : 0;
    }

    // 2. Run GPU kernel (computes vision, repulsion, movement)
    launch_matrix_ai_kernel(gpu_mats.data(), n, dt);

    // 3. Apply GPU results + CPU decisions
    for (int i = 0; i < n; i++) {
        MatrixGroup& m = all_matrices[i];
        const GPUMatrix& g = gpu_mats[i];

        m.lane_change_cooldown = g.out_lane_change_cooldown;

        // ---- LANE CHANGE / ASCENT Y-FIRST LOGIC ----
        // If currently transitioning to a new lane, freeze X and only move Y
        // While ascending, actively evade any matrix we pass through
        if (m.is_changing_lane) {
            float targetY = m.getLaneAltitude();
            float yDiff = std::abs(m.center.y - targetY);

            if (yDiff < 10.0f) {
                // Arrived at new lane altitude: resume normal X movement
                m.is_changing_lane = false;
                m.center.y = targetY;
                m.nav_target.y = targetY;
            } else {
                // Still transitioning: move only Y, keep X frozen
                m.center.y += (targetY - m.center.y) * 0.08f;
                m.nav_target.y = targetY;

                // MID-ASCENT COLLISION AVOIDANCE: check current position against ALL matrices
                float myHalfW = (m.cols * m.col_spacing) / 2.0f + MATRIX_SAFETY_MARGIN;
                float myHalfH = (m.rows * m.row_spacing) / 2.0f + MATRIX_SAFETY_MARGIN;

                for (int j = 0; j < n; j++) {
                    if (j == i) continue;
                    const MatrixGroup& other = all_matrices[j];
                    if (other.children.empty()) continue;

                    float oHalfW = (other.cols * other.col_spacing) / 2.0f + MATRIX_SAFETY_MARGIN;
                    float oHalfH = (other.rows * other.row_spacing) / 2.0f + MATRIX_SAFETY_MARGIN;

                    bool xOverlap = (m.center.x - myHalfW) < (other.center.x + oHalfW) &&
                                    (m.center.x + myHalfW) > (other.center.x - oHalfW);
                    bool yOverlap = (m.center.y - myHalfH) < (other.center.y + oHalfH) &&
                                    (m.center.y + myHalfH) > (other.center.y - oHalfH);

                    if (xOverlap && yOverlap) {
                        float dx = m.center.x - other.center.x;
                        float pushDir = (dx > 0.1f) ? 1.0f : (dx < -0.1f) ? -1.0f : (m.id > other.id ? 1.0f : -1.0f);
                        m.center.x += pushDir * COLLISION_REPULSION_FORCE * 3.0f;
                    }
                }

                m.center.x += g.out_repulsion_x;
                continue; // Skip all other logic for this frame
            }
        }

        // Apply GPU-computed movement
        m.center.x     = g.out_center_x;
        m.center.y     = g.out_center_y;
        m.nav_target.x = g.out_nav_target_x;
        m.nav_target.y = g.out_nav_target_y;

        // Apply cross-lane repulsion from GPU
        m.center.x += g.out_repulsion_x;

        // ---- STAGING ZONE REPULSION (CPU) ----
        // During TAKEOFF, push apart ALL overlapping matrices (any lane, any state)
        // GPU only does cross-lane repulsion; this catches same-slot staging overlaps
        if (m.current_action == toInt(DroneAction::TAKEOFF) && m.children.size() > 0) {
            float myHalfW = (m.cols * m.col_spacing) / 2.0f + MATRIX_SAFETY_MARGIN;
            float myHalfH = (m.rows * m.row_spacing) / 2.0f + MATRIX_SAFETY_MARGIN;

            for (int j = 0; j < n; j++) {
                if (j == i) continue;
                const MatrixGroup& other = all_matrices[j];
                if (other.children.empty()) continue;

                float oHalfW = (other.cols * other.col_spacing) / 2.0f + MATRIX_SAFETY_MARGIN;
                float oHalfH = (other.rows * other.row_spacing) / 2.0f + MATRIX_SAFETY_MARGIN;

                // AABB overlap check
                bool xOverlap = (m.center.x - myHalfW) < (other.center.x + oHalfW) &&
                                (m.center.x + myHalfW) > (other.center.x - oHalfW);
                bool yOverlap = (m.center.y - myHalfH) < (other.center.y + oHalfH) &&
                                (m.center.y + myHalfH) > (other.center.y - oHalfH);

                if (xOverlap && yOverlap) {
                    float dx = m.center.x - other.center.x;
                    // Push away from other matrix; tie-break by ID
                    float pushDir = (dx > 0.1f) ? 1.0f : (dx < -0.1f) ? -1.0f : (m.id > other.id ? 1.0f : -1.0f);
                    m.center.x += pushDir * COLLISION_REPULSION_FORCE * 2.0f;
                }
            }
        }

        // ---- FOLLOW_MATRIX: threat-based decisions ----
        if (m.current_action == toInt(DroneAction::FOLLOW_MATRIX)) {
            bool hasThreat = (g.out_threat_distance >= 0.0f);
            m.is_braking = false;

            if (hasThreat && m.lane_change_cooldown <= 0.0f) {
                if (g.out_threat_distance < FORWARD_VISION_DISTANCE * 0.4f) {
                    // Close threat: try lane change
                    int safeLane = findSafeLane(m, all_matrices);
                    if (safeLane != -1 && safeLane != m.assigned_lane) {
                        std::cout << "[MATRIX " << m.id << "] Lane change: "
                                  << m.assigned_lane << " -> " << safeLane << std::endl;
                        m.previous_lane = m.assigned_lane;
                        m.assigned_lane = safeLane;
                        m.nav_target.y  = m.getLaneAltitude();
                        m.is_changing_lane = true; // Freeze X until Y transition completes
                        m.lane_change_cooldown = 5.0f;
                        // Revert X to pre-GPU position (freeze during transition)
                        m.center.x = g.center_x + g.out_repulsion_x;
                    } else {
                        // All lanes blocked: brake
                        m.is_braking = true;
                        m.center.x = g.center_x + g.out_repulsion_x;
                        m.center.y = g.center_y;
                    }
                } else if (g.out_threat_distance < FORWARD_VISION_DISTANCE * 0.7f) {
                    // Medium threat: slow down
                    m.is_braking = true;
                    m.center.x = g.center_x + g.out_repulsion_x;
                    m.center.y = g.center_y;
                }
            } else if (hasThreat) {
                // Cooldown active: brake but check if path cleared
                m.is_braking = true;
                m.center.x = g.center_x + g.out_repulsion_x;
                m.center.y = g.center_y;
            }

            // STUCK RECOVERY: if braking but forward is now clear, resume
            if (m.is_braking && !hasThreat) {
                m.is_braking = false;
                // Let GPU movement apply (already set above before braking logic)
                m.center.x = g.out_center_x + g.out_repulsion_x;
                m.center.y = g.out_center_y;
            }
        }

        // ---- TAKEOFF -> FOLLOW_MATRIX promotion (staging -> highway ascent) ----
        if (m.current_action == toInt(DroneAction::TAKEOFF)) {
            float error = m.getGlobalError(drones);
            if (m.isFull() && error < MATRIX_ERROR_TOLERANCE) {
                // Check if highway lane is clear before ascending
                if (checkTakeoffClearance(m, all_matrices)) {
                    float highwayAlt = m.getLaneAltitude();
                    std::cout << "[MATRIX " << m.id << "] Formation stable in staging. "
                              << "Ascending to highway lane " << m.assigned_lane
                              << " (alt=" << highwayAlt << ")" << std::endl;
                    m.current_action = toInt(DroneAction::FOLLOW_MATRIX);
                    m.mission_timer  = 0.0f;
                    m.nav_target     = m.final_target;
                    m.nav_target.y   = highwayAlt;
                    // Trigger Y-first ascent: freeze X while climbing to highway altitude
                    m.is_changing_lane = true;
                }
            }
        }
    }
}