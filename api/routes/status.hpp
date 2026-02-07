#pragma once

#include "crow.h"
#include "../../services/infusion_supervisor.hpp"

inline void setup_status_routes(crow::SimpleApp& app, std::shared_ptr<InfusionSupervisor> supervisor)
{
    // ✅ API de status (usada pelo React)
    CROW_ROUTE(app, "/api/status")
    ([supervisor]() {
        auto t = supervisor->get_latest_telemetry();
        return crow::response(t.to_json().dump());
    });
}
