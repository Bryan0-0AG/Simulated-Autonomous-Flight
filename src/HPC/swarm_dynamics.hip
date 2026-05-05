#include <hip/hip_runtime.h>
#include <math.h>
#include "global_config.h"
#include "utils/vector2.h"
#include "HPC/swarm_dynamics.h"
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

    // Inicializamos constantes
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
    
    // 1. Integral con Anti-Windup (Saturación)
    prev_integral += error * dt;
    if (prev_integral > MAX_INTEGRAL) prev_integral = MAX_INTEGRAL;
    if (prev_integral < -MAX_INTEGRAL) prev_integral = -MAX_INTEGRAL;

    // 2. Derivativa
    float derivative = (error - prev_error) / dt;
    prev_error = error;

    // 3. Salida por componentes
    out.p = KP * error;
    out.i = KI * prev_integral;
    out.d = KD * derivative;

    return out;
}

__host__ __device__ ControlOutput compute_control(DroneChassis& drone, float dt) {
    ControlOutput output;

    // A. Calcular error actual usando aritmética de vectores
    drone.error = drone.target - drone.position;

    // B. Ejecutar PID para ÁNGULO
    PIDOutput angle_parts = compute_PID(
        dt,
        drone.error.x,
        drone.prev_error.x,
        drone.prev_integral.angle,
        false
    );

    // C. Ejecutar PID para EMPUJE (Thrust)
    PIDOutput thrust_parts = compute_PID(
        dt,
        drone.error.y,
        drone.prev_error.y,
        drone.prev_integral.thrust,
        true
    );

    // D. Combinar resultados
    output.angle  = angle_parts.p + angle_parts.i + angle_parts.d;

    float weight = drone.mass * GRAVITY;
    float thrust_adjustment = thrust_parts.p + thrust_parts.i + thrust_parts.d;
    output.thrust = weight + thrust_adjustment;

    // Límites de seguridad
    if (output.angle > MAX_ANGLE) output.angle = MAX_ANGLE;
    if (output.angle < MIN_ANGLE) output.angle = MIN_ANGLE;
    if (output.thrust > MAX_THRUST) output.thrust = MAX_THRUST;
    if (output.thrust < MIN_THRUST) output.thrust = MIN_THRUST;

    return output;
}

__host__ __device__ Vector2 compute_forces(DroneChassis& drone, float dt) {
    Vector2 forces = {0.0f, 0.0f};
    
    // 1. Gravedad (Tira hacia ABAJO: -Y en coordenadas cartesianas)
    forces.y -= (drone.mass * GRAVITY);
    
    // 2. Fricción
    forces += (drone.velocity * -AIR_FRICTION);
    
    // 3. Control (PID) - Solo si tiene bateria
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
    
    // 4. Separación (Viene de la CPU) - Sigue activa para evitar colisiones en cadena al caer
    forces += drone.f_separation;

    return forces;
}

__host__ __device__ void process_drone(DroneChassis& drone, float dt) {
    // 1. Calcular todas las fuerzas combinadas
    // Dentro de compute_forces, el dron solo generará Thrust si tiene batería
    Vector2 total_force = compute_forces(drone, dt);

    // 2. Integracion (Euler)
    Vector2 acceleration = total_force / drone.mass;
    drone.velocity += (acceleration * dt);

    // 3. Limite de Velocidad
    float speedSq = sq(drone.velocity.x) + sq(drone.velocity.y);
    if (speedSq > sq(MAX_VELOCITY)) {
        float speed = sqrtf(speedSq);
        drone.velocity = (drone.velocity / speed) * MAX_VELOCITY;
    }

    // 4. Actualizar Posicion
    drone.position += (drone.velocity * dt); 

    // 5. Consumo de Bateria (Solo si está volando o cayendo con batería)
    if (drone.battery > 0.0f) {
        // Solo consume si no está apoyado en el suelo o si está intentando despegar
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

// 3. FUNCIÓN DE HOST
extern "C" void launch_physics_kernel(DroneChassis* cpu_drones, int n, float dt) {
    DroneChassis* gpu_drones;
    size_t size = n * sizeof(DroneChassis);

    hipError_t err = hipMalloc(&gpu_drones, size);
    if (err != hipSuccess) {
        static bool warned = false;
        if (!warned) {
            printf("\n[FALLBACK CPU] Hardware ROCm AMD no detectado o sin memoria.\n");
            printf("[FALLBACK CPU] Transfiriendo todos los calculos fisicos de los drones al procesador (i3) de forma transparente...\n\n");
            warned = true;
        }
        // FALLBACK: Ejecutar la MISMA lógica del kernel en el host (CPU)
        for(int i = 0; i < n; i++) {
            process_drone(cpu_drones[i], dt);
        }
        return;
    }
    
    hipMemcpy(gpu_drones, cpu_drones, size, hipMemcpyHostToDevice);

    int blockSize = 256;
    int gridSize = (n + blockSize - 1) / blockSize;

    hipLaunchKernelGGL(massiveCompute_DRONES, 
                       dim3(gridSize), dim3(blockSize), 0, 0, 
                       gpu_drones, n, dt);

    err = hipGetLastError();
    if (err != hipSuccess) {
        printf("\n[ERROR CRITICO GPU] Fallo en Kernel: %s\n", hipGetErrorString(err));
    }

    hipDeviceSynchronize();
    hipMemcpy(cpu_drones, gpu_drones, size, hipMemcpyDeviceToHost);
    hipFree(gpu_drones);
}

#ifdef _WIN32
// Truco para engañar al linker y resolver el símbolo de borrado de MSVC
extern "C" void msvc_delete_hack(void* p, unsigned long long n) asm("??3@YAXPEAX_K@Z");
void msvc_delete_hack(void* p, unsigned long long n) {
    // Vacío: solo para que g++ no se queje
}
#endif
