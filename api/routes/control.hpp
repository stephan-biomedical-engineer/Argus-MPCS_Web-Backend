#pragma once

#include "crow.h"
#include "../../drivers/mqtt_client.hpp"
#include <thread>
#include <chrono>

inline void setup_control_routes(crow::SimpleApp& app, MqttClient* mqtt_client)
{
    CROW_ROUTE(app, "/api/control").methods(crow::HTTPMethod::POST)([mqtt_client](const crow::request& req) {
        auto x = crow::json::load(req.body);
        if(!x)
            return crow::response(400, "Invalid JSON");

        std::string action = x["action"].s();

        if(action == "start" && x.has("volume") && x.has("rate"))
        {
            crow::json::wvalue cfg;
            cfg["action"] = "config";
            cfg["volume"] = x["volume"].i();
            cfg["rate"] = x["rate"].i();
            // cfg["mode"] = x.has("mode") ? x["mode"].i() : 0;

            mqtt_client->publish("bomba/comando", cfg.dump());

            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            mqtt_client->publish("bomba/comando", R"({"action":"start"})");

            return crow::response(200, "Config + Start enviados");
        }

        mqtt_client->publish("bomba/comando", req.body);

        return crow::response(200, "Comando enviado");
    });
}
