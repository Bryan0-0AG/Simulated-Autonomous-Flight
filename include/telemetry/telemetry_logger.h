#ifndef TELEMETRY_LOGGER_H
#define TELEMETRY_LOGGER_H

#include <fstream>
#include <string>
#include <filesystem>
#include <ctime>
#include <sstream>
#include <iomanip>

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
        file << "time,id,pos_x,pos_y,vel_x,vel_y\n";
    }

    void log(float time, int id, float px, float py, float vx, float vy) {
        file << time << ","
             << id << ","
             << px << ","
             << py << ","
             << vx << ","
             << vy << "\n";
    }

    ~TelemetryLogger() {
        if (file.is_open()) file.close();
    }
};

#endif