#ifndef TELEMETRY_LOGGER_H
#define TELEMETRY_LOGGER_H

#include <fstream>
#include <string>
#include <filesystem>
#include <ctime>
#include <sstream>
#include <iomanip>

#include "global_config.h"
#include "AI/drone_states.h"
#include "drone_dynamics.h"
#include "AI/matrix/matrix_group.h"
#include "AI/matrix/properties.h"
#include "AI/matrix/states.h"

class TelemetryLogger {
private:
    std::ofstream file_all;

public:
    TelemetryLogger() {
        // Create folder with timestamp for each simulation
        std::time_t t = std::time(nullptr);
        std::tm* now = std::localtime(&t);
        std::stringstream ss;
        ss << "telemetry/logs/" << std::put_time(now, "%Y-%m-%d_%H-%M-%S");
        std::string dir = ss.str();
        std::filesystem::create_directories(dir);

        file_all.open(dir + "/Full_Telemetry.csv");

        // Header cleaned of fields not calculated on the GPU
        file_all << "time,num_drones,id,pos_x,pos_y,vel_x,vel_y,"
                 << "target_x,target_y,thrust_val,angle_val,"
                 << "error_x,error_y,f_sep_x,f_sep_y,"
                 << "action,state,battery,childs\n";
    }

    void log(float time, int num_drones, const MatrixGroup& matrix, const std::vector<DroneChassis>& drones) {
        Vector2 avg_pos = MatrixAI::Properties::getAveragePosition(matrix, drones);
        Vector2 avg_vel = MatrixAI::Properties::getAverageVelocity(matrix, drones);
        float avg_battery = MatrixAI::Properties::getAverageBattery(matrix, drones);
        Vector2 avg_repulsion = MatrixAI::Properties::getAverageRepulsionForce(matrix, drones);
        float avg_thrust = MatrixAI::Properties::getAverageThrust(matrix, drones);
        float avg_angle = MatrixAI::Properties::getAverageAngle(matrix, drones);
        Vector2 avg_error = MatrixAI::Properties::getVectorError(matrix, drones);

        file_all << time << ","
                 << num_drones << ","
                 << matrix.id << ","
                 << avg_pos.x << "," << avg_pos.y << ","
                 << avg_vel.x << "," << avg_vel.y << ","
                 << matrix.current_target.x << "," << matrix.current_target.y << ","
                 << avg_thrust << "," << avg_angle << ","
                 << avg_error.x << "," << avg_error.y << ","
                 << avg_repulsion.x << "," << avg_repulsion.y << ","
                 << MatrixAI::States::toString(MatrixAI::States::toAction(matrix.current_action)) << "," 
                 << MatrixAI::States::toString(MatrixAI::States::toState(matrix.current_state)) << ","
                 << avg_battery << ","
                 << matrix.children.size() << "\n";

        file_all.flush();
    }

    ~TelemetryLogger() {
        if (file_all.is_open()) file_all.close();
    }
};

#endif
