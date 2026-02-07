#pragma once

#include <string>
#include <memory>
#include "crow.h"

class InfusionSupervisor;
class MqttClient;

class HttpServer
{
private:
    crow::SimpleApp app_;

    std::shared_ptr<InfusionSupervisor> supervisor_;
    MqttClient* mqtt_client_;

    void setup_routes(const std::string& db_path);

public:
    HttpServer(std::shared_ptr<InfusionSupervisor>, MqttClient*, const std::string& db_path);

    void run(int port);
    void stop();
};
