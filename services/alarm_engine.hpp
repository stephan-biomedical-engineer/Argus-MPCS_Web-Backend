#pragma once
#include <string>
#include <vector>
#include "../models/telemetry.hpp"

class AlarmEngine
{
public:
    struct Alarm
    {
        std::string code;
        std::string message;
        bool is_critical;
    };

    std::vector<Alarm> check_anomalies(const Telemetry& t);
};
