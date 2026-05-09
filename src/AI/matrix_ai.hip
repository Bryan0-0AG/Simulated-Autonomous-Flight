#include <hip/hip_runtime.h>
#include <math.h>
#include "global_config.h"
#include "swarm/swarm_dynamics.h"  // GPUMatrix, constants

// DroneAction enum values as constants (mirrors states.h, GPU-compatible)
// TAKEOFF=0, FOLLOW_MATRIX=1 - must stay in sync with DroneAction enum order
constexpr int ACTION_TAKEOFF       = 0;
constexpr int ACTION_FOLLOW_MATRIX = 1;

// ============================================================
// DEVICE HELPERS (pure math, no branching)
// ============================================================

__host__ __device__ float matrix_half_width(const GPUMatrix& m) {
    return (m.cols * m.col_spacing) / 2.0f + MATRIX_SAFETY_MARGIN;
}

__host__ __device__ float matrix_half_height(const GPUMatrix& m) {
    return (m.rows * m.row_spacing) / 2.0f + MATRIX_SAFETY_MARGIN;
}

__host__ __device__ float lane_altitude(int lane) {
    return CRUISE_ALTITUDE_BASE + (lane * LANE_HEIGHT);
}

__host__ __device__ float safe_sign(float dx, int id_a, int id_b) {
    if (dx > 0.0f)  return  1.0f;
    if (dx < 0.0f)  return -1.0f;
    return (id_a > id_b) ? 1.0f : -1.0f;
}

// ============================================================
// FORWARD VISION (GPU-friendly: single loop, no early exit)
// Scans all matrices, returns distance to closest ahead threat.
// Returns -1.0f if path is clear.
// ============================================================
__host__ __device__ float forwardVisionDist(
    const GPUMatrix& me,
    const GPUMatrix* all,
    int n
) {
    float dirX = me.nav_target_x - me.center_x;
    float absDirX = (dirX < 0.0f) ? -dirX : dirX;
    if (absDirX < 10.0f) return -1.0f; // Barely moving

    float sign = (dirX > 0.0f) ? 1.0f : -1.0f;
    float closest = FORWARD_VISION_DISTANCE + 1.0f;

    float myMinY = me.center_y - matrix_half_height(me) - LANE_HEIGHT;
    float myMaxY = me.center_y + matrix_half_height(me) + LANE_HEIGHT;

    for (int i = 0; i < n; i++) {
        if (all[i].id == me.id) continue;
        if (all[i].child_count == 0) continue;

        float dxToOther = all[i].center_x - me.center_x;
        // Is it ahead of us? (dot product with direction)
        float projected = sign * dxToOther;
        if (projected <= 0.0f) continue;

        float dist = (dxToOther < 0.0f) ? -dxToOther : dxToOther;
        if (dist >= FORWARD_VISION_DISTANCE) continue;

        // Check Y overlap (same or adjacent lane zone)
        float thMinY = all[i].center_y - matrix_half_height(all[i]);
        float thMaxY = all[i].center_y + matrix_half_height(all[i]);
        bool yOverlap = !(thMaxY < myMinY || thMinY > myMaxY);

        // Use min() instead of branching - GPU-friendly
        if (yOverlap && dist < closest) {
            closest = dist;
        }
    }
    return (closest <= FORWARD_VISION_DISTANCE) ? closest : -1.0f;
}

// ============================================================
// CROSS-LANE AABB REPULSION (GPU-friendly: single loop, accumulate)
// ============================================================
__host__ __device__ float crossLaneRepulsionX(
    const GPUMatrix& me,
    const GPUMatrix* all,
    int n
) {
    float repX = 0.0f;
    float myMinX = me.center_x - matrix_half_width(me);
    float myMaxX = me.center_x + matrix_half_width(me);
    float myMinY = me.center_y - matrix_half_height(me);
    float myMaxY = me.center_y + matrix_half_height(me);

    for (int i = 0; i < n; i++) {
        if (all[i].id == me.id) continue;
        if (all[i].child_count == 0) continue;
        if (all[i].assigned_lane == me.assigned_lane) continue;

        float oMinX = all[i].center_x - matrix_half_width(all[i]);
        float oMaxX = all[i].center_x + matrix_half_width(all[i]);
        float oMinY = all[i].center_y - matrix_half_height(all[i]);
        float oMaxY = all[i].center_y + matrix_half_height(all[i]);

        bool overlap = (myMinX < oMaxX && myMaxX > oMinX &&
                        myMinY < oMaxY && myMaxY > oMinY);
        if (overlap) {
            float dx = me.center_x - all[i].center_x;
            repX += safe_sign(dx, me.id, all[i].id) * COLLISION_REPULSION_FORCE;
        }
    }
    return repX;
}

// ============================================================
// MAIN KERNEL - one thread per matrix
// GPU computes: forward vision distance, repulsion forces, movement.
// GPU does NOT decide lane changes (that branching logic stays on CPU).
// ============================================================
__global__ void massiveCompute_MATRICES(GPUMatrix* mats, int n, float dt) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= n) return;

    GPUMatrix me = mats[idx]; // Local copy for reading

    // Copy inputs to outputs as default (no change)
    me.out_center_x             = me.center_x;
    me.out_center_y             = me.center_y;
    me.out_nav_target_x         = me.nav_target_x;
    me.out_nav_target_y         = me.nav_target_y;
    me.out_assigned_lane        = me.assigned_lane;
    me.out_lane_change_cooldown = me.lane_change_cooldown;
    me.out_is_braking           = 0;
    me.out_threat_distance      = -1.0f;
    me.out_repulsion_x          = 0.0f;

    // ---- TAKEOFF: only emergency cross-lane nudge ----
    if (me.current_action == ACTION_TAKEOFF) {
        if (me.child_count > 0) {
            me.out_repulsion_x = crossLaneRepulsionX(me, mats, n);
            me.out_center_x += me.out_repulsion_x * 0.1f;
        }
        mats[idx] = me;
        return;
    }

    // ---- Only process FOLLOW_MATRIX ----
    if (me.current_action != ACTION_FOLLOW_MATRIX) {
        mats[idx] = me;
        return;
    }

    // 1. Cooldown tick
    if (me.lane_change_cooldown > 0.0f)
        me.out_lane_change_cooldown = me.lane_change_cooldown - dt;

    // 2. Forward vision - just compute the distance (CPU decides what to do)
    me.out_threat_distance = forwardVisionDist(me, mats, n);

    // 3. Cross-lane repulsion - compute the force (CPU applies it)
    me.out_repulsion_x = crossLaneRepulsionX(me, mats, n);

    // 4. Advance center toward nav_target (pure math, no branching decisions)
    // Note: CPU may override this by setting is_braking after lane change decision
    float dxNav = me.nav_target_x - me.center_x;
    float dyNav = me.nav_target_y - me.center_y;
    float distNav = sqrtf(dxNav * dxNav + dyNav * dyNav);
    float step = MATRIX_MOVE_SPEED * dt;

    if (distNav > step) {
        float ratio = step / distNav;
        me.out_center_x = me.center_x + dxNav * ratio;
        me.out_center_y = me.center_y + dyNav * ratio;
    } else {
        me.out_center_x = me.nav_target_x;
        me.out_center_y = me.nav_target_y;
    }

    // 5. Lane altitude drift correction
    float targetY = lane_altitude(me.out_assigned_lane);
    float yDiff = me.out_nav_target_y - targetY;
    if (yDiff > 5.0f || yDiff < -5.0f) {
        me.out_nav_target_y += (targetY - me.out_nav_target_y) * 0.05f;
    }

    // Write back
    mats[idx] = me;
}

// ============================================================
// HOST LAUNCHER (same GPU/CPU fallback pattern as swarm_dynamics)
// ============================================================
static void process_matrix_cpu(GPUMatrix& me, const GPUMatrix* all, int n, float dt) {
    me.out_center_x             = me.center_x;
    me.out_center_y             = me.center_y;
    me.out_nav_target_x         = me.nav_target_x;
    me.out_nav_target_y         = me.nav_target_y;
    me.out_assigned_lane        = me.assigned_lane;
    me.out_lane_change_cooldown = me.lane_change_cooldown;
    me.out_is_braking           = 0;
    me.out_threat_distance      = -1.0f;
    me.out_repulsion_x          = 0.0f;

    if (me.current_action == ACTION_TAKEOFF) {
        me.out_repulsion_x = crossLaneRepulsionX(me, all, n);
        me.out_center_x += me.out_repulsion_x * 0.1f;
        return;
    }
    if (me.current_action != ACTION_FOLLOW_MATRIX) return;

    if (me.lane_change_cooldown > 0.0f)
        me.out_lane_change_cooldown = me.lane_change_cooldown - dt;

    me.out_threat_distance = forwardVisionDist(me, all, n);
    me.out_repulsion_x = crossLaneRepulsionX(me, all, n);

    float dxNav = me.nav_target_x - me.center_x;
    float dyNav = me.nav_target_y - me.center_y;
    float distNav = sqrtf(dxNav * dxNav + dyNav * dyNav);
    float step = MATRIX_MOVE_SPEED * dt;

    if (distNav > step) {
        float ratio = step / distNav;
        me.out_center_x = me.center_x + dxNav * ratio;
        me.out_center_y = me.center_y + dyNav * ratio;
    } else {
        me.out_center_x = me.nav_target_x;
        me.out_center_y = me.nav_target_y;
    }

    float targetY = lane_altitude(me.out_assigned_lane);
    float yDiff = me.out_nav_target_y - targetY;
    if (yDiff > 5.0f || yDiff < -5.0f)
        me.out_nav_target_y += (targetY - me.out_nav_target_y) * 0.05f;
}

extern "C" void launch_matrix_ai_kernel(GPUMatrix* cpu_mats, int n, float dt) {
    if (n == 0) return;

    GPUMatrix* gpu_mats;
    size_t size = n * sizeof(GPUMatrix);

    hipError_t err = hipMalloc(&gpu_mats, size);
    if (err != hipSuccess) {
        // CPU fallback
        static bool warned = false;
        if (!warned) {
            printf("[MATRIX AI] GPU unavailable, running on CPU fallback.\n");
            warned = true;
        }
        for (int i = 0; i < n; i++)
            process_matrix_cpu(cpu_mats[i], cpu_mats, n, dt);
        return;
    }

    hipMemcpy(gpu_mats, cpu_mats, size, hipMemcpyHostToDevice);

    int blockSize = 64;
    int gridSize  = (n + blockSize - 1) / blockSize;

    hipLaunchKernelGGL(massiveCompute_MATRICES,
                       dim3(gridSize), dim3(blockSize), 0, 0,
                       gpu_mats, n, dt);

    hipError_t kerr = hipGetLastError();
    if (kerr != hipSuccess)
        printf("[MATRIX AI] Kernel error: %s\n", hipGetErrorString(kerr));

    hipDeviceSynchronize();
    hipMemcpy(cpu_mats, gpu_mats, size, hipMemcpyDeviceToHost);
    hipFree(gpu_mats);
}
