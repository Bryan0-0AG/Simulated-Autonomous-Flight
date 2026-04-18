#ifndef TELEMETRY_LOGGER_H
#define TELEMETRY_LOGGER_H

#include <fstream>
#include <string>
#include <filesystem>
#include <ctime>
#include <sstream>
#include <iomanip>

#include "control/control_config.h"

class TelemetryLogger {
private:
    std::ofstream file_physics;
    std::ofstream file_control;
    std::ofstream file_ai;
    std::ofstream file_all;

public:
    TelemetryLogger() {
        std::string dir = "telemetry/logs/Actual Simulation";
        std::filesystem::create_directories(dir);

        file_physics.open(dir + "/Physics.csv");
        file_control.open(dir + "/Control.csv");
        file_ai.open(dir + "/AI.csv");
        file_all.open(dir + "/All.csv");

        file_physics << "time,id,pos_x,pos_y,vel_x,vel_y\n";
        file_control << "time,id,target_x,target_y,thrust,angle,error_x,error_y,angle_pid_p,angle_pid_i,angle_pid_d,thrust_pid_p,thrust_pid_i,thrust_pid_d\n";
        file_ai << "time,id,action,state,battery,max_battery,original_target_x,original_target_y\n";
        file_all << "time,id,pos_x,pos_y,vel_x,vel_y,target_x,target_y,thrust,angle,error_x,error_y,angle_pid_p,angle_pid_i,angle_pid_d,thrust_pid_p,thrust_pid_i,thrust_pid_d,action,state,battery,max_battery,original_target_x,original_target_y\n";
    }

    void log(
        float time, 
        int id, 
        float px, float py, 
        float vx, float vy, 
        float tx, float ty,
        float thrust, float angle,
        float error_x, float error_y,
        PIDOutput angle_pid, PIDOutput thrust_pid,
        std::string action, std::string state,
        float battery, float max_battery,
        float original_target_x, float original_target_y
    ) {
        file_physics << time << "," << id << "," << px << "," << py << "," << vx << "," << vy << "\n";

        file_control << time << "," << id << "," << tx << "," << ty << ","
                     << thrust << "," << angle << "," << error_x << "," << error_y << ","
                     << angle_pid.p << "," << angle_pid.i << "," << angle_pid.d << ","
                     << thrust_pid.p << "," << thrust_pid.i << "," << thrust_pid.d << "\n";

        file_ai << time << "," << id << "," << action << "," << state << ","
                << battery << "," << max_battery << ","
                << original_target_x << "," << original_target_y << "\n";

        file_all << time << ","
                 << id << ","
                 << px << ","
                 << py << ","
                 << vx << ","
                 << vy << ","
                 << tx << ","
                 << ty << ","
                 << thrust << ","
                 << angle << ","
                 << error_x << ","
                 << error_y << ","
                 << angle_pid.p << ","
                 << angle_pid.i << ","
                 << angle_pid.d << ","
                 << thrust_pid.p << ","
                 << thrust_pid.i << ","
                 << thrust_pid.d << ","
                 << action << ","
                 << state << ","
                 << battery << ","
                 << max_battery << ","
                 << original_target_x << ","
                 << original_target_y << "\n";

        file_physics.flush();
        file_control.flush();
        file_ai.flush();
        file_all.flush();
    }

    ~TelemetryLogger() {
        if (file_physics.is_open()) file_physics.close();
        if (file_control.is_open()) file_control.close();
        if (file_ai.is_open()) file_ai.close();
        if (file_all.is_open()) file_all.close();
    }
};

#endif