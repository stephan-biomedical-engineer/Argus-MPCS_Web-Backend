#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <chrono>

using json = nlohmann::json;

struct Telemetry
{
    // Campos espelhados do firmware STM32/Python
    std::string state; // "RUNNING", "STOPPED", "ALARM"
    double volume_infused_ml;
    double real_rate_ml_h;
    bool alarm_active;
    long long timestamp; // Unix timestamp

    // Converter OBJETO C++ -> JSON (Para salvar no banco ou mandar pra API)
    json to_json() const
    {
        return json{{"state", state},
                    {"infused_volume_ml", volume_infused_ml},
                    {"real_rate_ml_h", real_rate_ml_h},
                    {"alarm_active", alarm_active},
                    {"timestamp", timestamp}};
    }

    // Converter JSON -> OBJETO C++ (Quando chega do MQTT)
    static Telemetry from_json(const json& j)
    {
        Telemetry t;
        // O .value() é seguro: se o campo não existir, usa o valor padrão (o segundo argumento)
        t.state = j.value("state", "UNKNOWN");
        t.volume_infused_ml = j.value("infused_volume_ml", 0.0);
        t.real_rate_ml_h = j.value("real_rate_ml_h", 0.0);

        // Lógica simples para detectar alarme se vier como string "ALARM" ou bool
        if(j.contains("state") && j["state"] == "ALARM")
        {
            t.alarm_active = true;
        }
        else
        {
            t.alarm_active = j.value("alarm_active", false);
        }

        // Pega timestamp atual se não vier no JSON
        if(j.contains("timestamp"))
        {
            t.timestamp = j["timestamp"];
        }
        else
        {
            t.timestamp =
                std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
                    .count();
        }

        return t;
    }
};
