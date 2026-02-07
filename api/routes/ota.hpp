#pragma once

#include <crow.h>
#include <fstream>
#include <cstdlib>

static const std::string RPI_USER = "root";
//static const std::string RPI_IP   = "192.168.1.6";
static const std::string RPI_IP   = "argus-pump.local";

inline void setup_ota_routes(crow::SimpleApp& app, MqttClient* mqtt)
{
    CROW_ROUTE(app, "/api/ota/upload")
        .methods("POST"_method)
    ([mqtt](const crow::request& req)
    {
        // ==========================================
        // Nome do arquivo vindo no header
        // ==========================================
        std::string filename = req.get_header_value("X-Filename");

        if (filename.empty())
            return crow::response(400, "Filename ausente");

        std::string firmware_path = "/tmp/" + filename;

        // ==========================================
        // Salva binário bruto
        // ==========================================
        std::ofstream ofs(firmware_path, std::ios::binary);
        ofs.write(req.body.c_str(), req.body.size());
        ofs.close();

        if (!std::filesystem::exists(firmware_path))
            return crow::response(500, "Falha ao salvar firmware");

        // ==========================================
        // SCP → Raspberry
        // ==========================================
        std::string scp_cmd =
            "scp -o StrictHostKeyChecking=no "
            "-o UserKnownHostsFile=/dev/null "
            + firmware_path + " "
            + RPI_USER + "@" + RPI_IP + ":" + firmware_path;

        int ret = std::system(scp_cmd.c_str());

        std::cout << "[OTA] SCP CMD: " << scp_cmd << std::endl;
        std::cout << "[OTA] RET: " << ret << std::endl;

        if (ret != 0)
            return crow::response(500, "Erro ao enviar firmware via SCP");

        // ==========================================
        // MQTT trigger
        // ==========================================
        std::string payload =
            "{\"action\":\"update_firmware\",\"file_path\":\"" +
            firmware_path + "\"}";

        mqtt->publish("bomba/comando", payload);

        return crow::response(200, firmware_path);
    });
}
