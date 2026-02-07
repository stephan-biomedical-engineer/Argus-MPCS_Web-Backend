#include "alarm_engine.hpp"

std::vector<AlarmEngine::Alarm> AlarmEngine::check_anomalies(const Telemetry& t)
{
    std::vector<Alarm> alarms;

    // Regra: Se diz RUNNING mas a vazão é 0, algo travou
    if(t.state == "RUNNING" && t.real_rate_ml_h < 0.1)
    {
        alarms.push_back({"ERR01", "Motor travado ou oclusao", true});
    }

    // Regra: Se o hardware reportou erro
    if(t.state == "ALARM")
    {
        alarms.push_back({"ERR02", "Hardware Error", true});
    }

    return alarms;
}
