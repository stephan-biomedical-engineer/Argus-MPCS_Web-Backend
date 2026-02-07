#include "http_server.hpp"
#include <iostream>
#include <filesystem>
#include "routes/status.hpp"
#include "routes/control.hpp"
#include "routes/sessions.hpp"
#include "routes/ota.hpp"

#include <fstream>
#include <sstream>

HttpServer::HttpServer(std::shared_ptr<InfusionSupervisor> supervisor, MqttClient* mqtt, const std::string& db_path)
    : supervisor_(supervisor), mqtt_client_(mqtt)
{
    setup_routes(db_path);
}

void HttpServer::setup_routes(const std::string& db_path)
{
    setup_status_routes(app_, supervisor_);
    setup_control_routes(app_, mqtt_client_);
    setup_session_routes(app_, db_path);
    setup_ota_routes(app_, mqtt_client_);
}

void HttpServer::run(int port)
{
    app_.loglevel(crow::LogLevel::Warning);
    app_.signal_clear();

    auto ends_with = [](const std::string& s, const std::string& suffix) {
        return s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
    };

    CROW_ROUTE(app_, "/")
    ([]() {
        std::ifstream file("static/index.html");
        if(!file.is_open())
            return crow::response(500, "index.html nao encontrado");

        std::stringstream ss;
        ss << file.rdbuf();

        crow::response res(ss.str());
        res.set_header("Content-Type", "text/html; charset=utf-8");
        return res;
    });

    CROW_ROUTE(app_, "/assets/<path>")
    ([ends_with](const std::string& path) {
        std::ifstream file("static/assets/" + path, std::ios::binary);
        if(!file.is_open())
            return crow::response(404);

        std::ostringstream ss;
        ss << file.rdbuf();

        crow::response res(ss.str());

        if(ends_with(path, ".js"))
            res.set_header("Content-Type", "application/javascript");
        else if(ends_with(path, ".css"))
            res.set_header("Content-Type", "text/css");
        else if(ends_with(path, ".svg"))
            res.set_header("Content-Type", "image/svg+xml");
        else if(ends_with(path, ".json"))
            res.set_header("Content-Type", "application/json");
        else
            res.set_header("Content-Type", "application/octet-stream");

        return res;
    });

    std::cout << "[WEB] Backend iniciado na porta " << port << std::endl;
    app_.port(port).ssl_file("/app/certs/cert.pem", "/app/certs/key.pem").run();
    //app_.port(port).run();
}

void HttpServer::stop()
{
    app_.stop();
}
