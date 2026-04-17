#ifndef TELEMETRY_LOGGER_H
#define TELEMETRY_LOGGER_H

#include <fstream>
#include <string>
#include <filesystem>
#include <ctime>
#include <sstream>
#include <iomanip>

#include "vehicle/control_config.h"

class TelemetryLogger {
private:
    std::ofstream file;

    std::string getDateString() {
        std::time_t t = std::time(nullptr);
        std::tm tm = *std::localtime(&t);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
        return oss.str();
    }

public:
    TelemetryLogger() {
        std::string dir = "telemetry/logs";
        std::filesystem::create_directories(dir);

        std::string filename = dir + "/telemetry_" + getDateString() + ".csv";

        file.open(filename);
        file << "time,id,pos_x,pos_y,vel_x,vel_y,target_x,target_y,thrust,angle,error_x,error_y,angle_pid_p,angle_pid_i,angle_pid_d,thrust_pid_p,thrust_pid_i,thrust_pid_d\n";
    }

    void log(
        float time, 
        int id, 
        float px, float py, 
        float vx, float vy, 
        float tx, float ty,
        float thrust, float angle,
        float error_x, float error_y,
        PIDOutput angle_pid, PIDOutput thrust_pid
    ) {
        file << time << ","
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
             << thrust_pid.d << "\n";
    }

    ~TelemetryLogger() {
        if (file.is_open()) file.close();
    }
};

#endif