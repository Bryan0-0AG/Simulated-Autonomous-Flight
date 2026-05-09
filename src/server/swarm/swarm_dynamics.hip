#include <hip/hip_runtime.h>
#include <math.h>
#include "global_config.h"
#include "utils/vector2.h"
#include "swarm/swarm_dynamics.h"
__host__ __device__ float sq(float base){
    return base * base;
}


__host__ __device__ PIDOutput compute_PID(
    float dt,
    float error,
    float& prev_error,
    float& prev_integral,
    bool is_thrust
) {
    PIDOutput out;

    // Initialize constants
    float KP, KI, KD, MAX_INTEGRAL;
    if (is_thrust) {
        KP = PID_KP_THRUST;
        KI = PID_KI_THRUST;
        KD = PID_KD_THRUST;
        MAX_INTEGRAL = PID_MAX_I_THRUST;
    } else {
        KP = PID_KP_ANGLE;
        KI = PID_KI_ANGLE;
        KD = PID_KD_ANGLE;
        MAX_INTEGRAL = PID_MAX_I_ANGLE;
    }
    
    // 1. Integral with Anti-Windup (Saturation)
    prev_integral += error * dt;
    if (prev_integral > MAX_INTEGRAL) prev_integral = MAX_INTEGRAL;
    if (prev_integral < -MAX_INTEGRAL) prev_integral = -MAX_INTEGRAL;

    // 2. Derivative
    float derivative = (error - prev_error) / dt;
    prev_error = error;

    // 3. Component output
    out.p = KP * error;
    out.i = KI * prev_integral;
    out.d = KD * derivative;

    return out;
}

__host__ __device__ ControlOutput compute_control(DroneChassis& drone, float dt) {
    ControlOutput output;

    // A. Calculate current error using vector arithmetic
    drone.error = drone.target - drone.position;

    // B. Execute PID for ANGLE
    PIDOutput angle_parts = compute_PID(
        dt,
        drone.error.x,
        drone.prev_error.x,
        drone.prev_integral.angle,
        false
    );

    // C. Execute PID for THRUST
    PIDOutput thrust_parts = compute_PID(
        dt,
        drone.error.y,
        drone.prev_error.y,
        drone.prev_integral.thrust,
        true
    );

    // D. Combine results
    output.angle  = angle_parts.p + angle_parts.i + angle_parts.d;

    float weight = drone.mass * GRAVITY;
    float thrust_adjustment = thrust_parts.p + thrust_parts.i + thrust_parts.d;
    output.thrust = weight + thrust_adjustment;

    // Safety limits
    if (output.angle > MAX_ANGLE) output.angle = MAX_ANGLE;
    if (output.angle < MIN_ANGLE) output.angle = MIN_ANGLE;
    if (output.thrust > MAX_THRUST) output.thrust = MAX_THRUST;
    if (output.thrust < MIN_THRUST) output.thrust = MIN_THRUST;

    return output;
}

__host__ __device__ Vector2 compute_forces(DroneChassis& drone, float dt) {
    Vector2 forces = {0.0f, 0.0f};
    
    // 1. Gravity (Pulls DOWN: -Y in Cartesian coordinates)
    forces.y -= (drone.mass * GRAVITY);
    
    // 2. Air Friction
    forces += (drone.velocity * -AIR_FRICTION);
    
    // 3. Control (PID) - Only if battery is available
    //if (drone.battery > 0.0f) {
        drone.control_output = compute_control(drone, dt);
        
        Vector2 thrust_vec = {
            drone.control_output.thrust * sinf(drone.control_output.angle),
            drone.control_output.thrust * cosf(drone.control_output.angle) 
        };
        forces += thrust_vec;
    //} else {
    //    drone.control_output.thrust = 0.0f;
    //}
    
    // 4. Separation (From CPU) - Remains active to avoid chain collisions when falling
    forces += drone.f_separation;

    return forces;
}

__host__ __device__ void process_drone(DroneChassis& drone, float dt) {
    // 1. Calculate all combined forces
    // Within compute_forces, the drone will only generate thrust if it has battery
    Vector2 total_force = compute_forces(drone, dt);

    // 2. Integration (Euler)
    Vector2 acceleration = total_force / drone.mass;
    drone.velocity += (acceleration * dt);

    // 3. Speed Limit
    float speedSq = sq(drone.velocity.x) + sq(drone.velocity.y);
    if (speedSq > sq(MAX_VELOCITY)) {
        float speed = sqrtf(speedSq);
        drone.velocity = (drone.velocity / speed) * MAX_VELOCITY;
    }

    // 4. Update Position
    drone.position += (drone.velocity * dt); 

    // 5. Battery Consumption (Only if flying or falling with battery)
    if (drone.battery > 0.0f) {
        // Only consumes if not resting on the ground or if trying to take off
        if (drone.position.y > 0.1f || drone.velocity.y > 0.1f) {
            drone.battery -= BATTERY_DRAIN * dt; 
            if (drone.battery < 0.0f) drone.battery = 0.0f;
        }
    }
}

__global__ void massiveCompute_DRONES(
    DroneChassis* drones, 
    int number_of_drones, 
    float dt
) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;

    if (idx < number_of_drones) {
        process_drone(drones[idx], dt);
    }
}

// 3. HOST FUNCTION
extern "C" void launch_physics_kernel(DroneChassis* cpu_drones, int n, float dt, const GPUObstacle* obstacles, int num_obstacles) {
    DroneChassis* gpu_drones;
    size_t size = n * sizeof(DroneChassis);

    hipError_t err = hipMalloc(&gpu_drones, size);
    if (err != hipSuccess) {
        static bool warned = false;
        if (!warned) {
            printf("\n[FALLBACK CPU] Hardware ROCm AMD not detected or out of memory.\n");
            printf("[FALLBACK CPU] Transferring all drone physical calculations to CPU transparently...\n\n");
            warned = true;
        }
        // FALLBACK: Execute the SAME kernel logic on the host (CPU)
        for(int i = 0; i < n; i++) {
            process_drone(cpu_drones[i], dt);
        }
        return;
    }
    
    (void)hipMemcpy(gpu_drones, cpu_drones, size, hipMemcpyHostToDevice);

    int blockSize = 256;
    int gridSize = (n + blockSize - 1) / blockSize;

    hipLaunchKernelGGL(massiveCompute_DRONES, 
                       dim3(gridSize), dim3(blockSize), 0, 0, 
                       gpu_drones, n, dt);

    err = hipGetLastError();
    if (err != hipSuccess) {
        printf("\n[CRITICAL GPU ERROR] Kernel Fail: %s\n", hipGetErrorString(err));
    }

    (void)hipDeviceSynchronize();
    (void)hipMemcpy(cpu_drones, gpu_drones, size, hipMemcpyDeviceToHost);
    (void)hipFree(gpu_drones);
}

#ifdef _WIN32
// Truco para engañar al linker y resolver el símbolo de borrado de MSVC
extern "C" void msvc_delete_hack(void* p, unsigned long long n) asm("??3@YAXPEAX_K@Z");
void msvc_delete_hack(void* p, unsigned long long n) {
    // Vacío: solo para que g++ no se queje
}
#endif
