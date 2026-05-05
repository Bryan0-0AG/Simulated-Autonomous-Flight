#ifndef TELEMETRY_LOGGER_H
#define TELEMETRY_LOGGER_H

#include <fstream>
#include <string>
#include <filesystem>
#include <ctime>
#include <sstream>
#include <iomanip>

#include "global_config.h"
#include "AI/states.h"
#include "HPC/swarm_dynamics.h"

class TelemetryLogger {
private:
    std::ofstream file_all;

public:
    TelemetryLogger() {
        // Crear carpeta con timestamp para cada simulaciÃƒÂ³n
        std::time_t t = std::time(nullptr);
        std::tm* now = std::localtime(&t);
        std::stringstream ss;
        ss << "telemetry/logs/" << std::put_time(now, "%Y-%m-%d_%H-%M-%S");
        std::string dir = ss.str();
        std::filesystem::create_directories(dir);

        file_all.open(dir + "/Full_Telemetry.csv");

        // Header limpio de campos no calculados en la GPU
        file_all << "time,num_drones,id,pos_x,pos_y,vel_x,vel_y,"
                 << "target_x,target_y,thrust_val,angle_val,"
                 << "error_x,error_y,f_sep_x,f_sep_y,"
                 << "action,state,battery\n";
    }

    void log(float time, int num_drones, const DroneChassis& d) {
        file_all << time << ","
                 << num_drones << ","
                 << d.id << ","
                 << d.position.x << "," << d.position.y << ","
                 << d.velocity.x << "," << d.velocity.y << ","
                 << d.target.x << "," << d.target.y << ","
                 << d.control_output.thrust << "," << d.control_output.angle << ","
                 << d.error.x << "," << d.error.y << ","
                 << d.f_separation.x << "," << d.f_separation.y << ","
                 << toString(d.current_action) << "," << toString(d.current_state) << ","
                 << d.battery << "\n";

        file_all.flush();
    }

    ~TelemetryLogger() {
        if (file_all.is_open()) file_all.close();
    }
};

#endif