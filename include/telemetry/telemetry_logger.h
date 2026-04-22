#ifndef TELEMETRY_LOGGER_H
#define TELEMETRY_LOGGER_H

#include <fstream>
#include <string>
#include <filesystem>
#include <ctime>
#include <sstream>
#include <iomanip>

#include "global_config.h"
#include "body.h"

class TelemetryLogger {
private:
    std::ofstream file_all;

public:
    TelemetryLogger() {
        // Crear carpeta con timestamp para cada simulación
        std::time_t t = std::time(nullptr);
        std::tm* now = std::localtime(&t);
        std::stringstream ss;
        ss << "telemetry/logs/" << std::put_time(now, "%Y-%m-%d_%H-%M-%S");
        std::string dir = ss.str();
        std::filesystem::create_directories(dir);

        file_all.open(dir + "/Full_Telemetry.csv");

        // Header extendido
        file_all << "time,num_drones,id,pos_x,pos_y,vel_x,vel_y,"
                 << "target_x,target_y,thrust_val,angle_val,"
                 << "error_x,error_y,angle_p,angle_i,angle_d,thrust_p,thrust_i,thrust_d,"
                 << "f_grav_x,f_grav_y,f_thrust_x,f_thrust_y,f_sep_x,f_sep_y,f_drag_x,f_drag_y,"
                 << "action,state,battery\n";
    }

    void log(float time, int num_drones, const Body& b) {
        file_all << time << ","
                 << num_drones << ","
                 << b.id << ","
                 << b.position.x << "," << b.position.y << ","
                 << b.velocity.x << "," << b.velocity.y << ","
                 << b.target.x << "," << b.target.y << ","
                 << b.actuator_output.thrust << "," << b.actuator_output.angle << ","
                 << b.error_x << "," << b.error_y << ","
                 << b.angle_pid.p << "," << b.angle_pid.i << "," << b.angle_pid.d << ","
                 << b.thrust_pid.p << "," << b.thrust_pid.i << "," << b.thrust_pid.d << ","
                 << b.f_gravity.x << "," << b.f_gravity.y << ","
                 << b.f_thrust.x << "," << b.f_thrust.y << ","
                 << b.f_separation.x << "," << b.f_separation.y << ","
                 << b.f_drag.x << "," << b.f_drag.y << ","
                 << (int)b.current_action << "," << (int)b.current_state << ","
                 << b.battery << "\n";

        file_all.flush();
    }

    ~TelemetryLogger() {
        if (file_all.is_open()) file_all.close();
    }
};

#endif