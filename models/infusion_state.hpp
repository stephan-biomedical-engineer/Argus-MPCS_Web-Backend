#pragma once
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct InfusionState
{
    std::string motor_status; // IDLE, RUNNING, KVO, BOLUS
    double current_volume_ml;
    double current_rate_ml_h;
    bool is_online;
    std::string last_error;

    json to_json() const
    {
        return json{{"state", motor_status},
                    {"vol", current_volume_ml},
                    {"rate", current_rate_ml_h},
                    {"online", is_online},
                    {"error", last_error}};
    }
};
