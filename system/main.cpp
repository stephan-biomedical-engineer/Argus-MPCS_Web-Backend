#include <iostream>
#include <fstream> // Ler arquivo
#include <thread>
#include <atomic>
#include <csignal>

#include "../drivers/database_driver.hpp"
#include "../drivers/mqtt_client.hpp"
#include "../services/session_manager.hpp"
#include "../services/alarm_engine.hpp"
#include "../services/infusion_supervisor.hpp"
#include "../api/http_server.hpp"
// [REMOVIDO] #include "../api/routes/sessions.hpp" -> Isso vai para dentro do http_server.cpp agora

#include <nlohmann/json.hpp>
using json = nlohmann::json;

std::atomic<bool> running(true);

void signal_handler(int)
{
    running = false;
}

json load_config()
{
    std::ifstream f("config.json");
    if(!f.is_open())
    {
        std::cerr << "[CONFIG] config.json nao encontrado! Usando padroes..." << std::endl;
        return {{"mqtt", {{"broker_ip", "localhost"}, {"broker_port", 8883}, {"client_id", "backend_cpp"}}},
                {"web", {{"port", 8080}}},
                {"database", {{"path", "historico_clinico.db"}}}};
    }
    return json::parse(f);
}

int main()
{
    std::signal(SIGINT, signal_handler);
    std::cout << "--- ESTACAO DE MONITORAMENTO EXTERNA ---" << std::endl;

    try
    {
        // 1. CARREGAR CONFIGURAÇÃO
        json config = load_config();

        std::string db_path = config["database"]["path"]; // <--- Já temos o caminho aqui
        std::string mqtt_ip = config["mqtt"]["broker_ip"];
        int mqtt_port = config["mqtt"]["broker_port"];
        std::string mqtt_id = config["mqtt"]["client_id"];
        int web_port = config["web"]["port"];

        // std::string mqtt_uri = "tcp://" + mqtt_ip + ":" + std::to_string(mqtt_port);
        std::string mqtt_uri = "ssl://" + mqtt_ip + ":" + std::to_string(mqtt_port);
        std::cout << "[CONFIG] Alvo: " << mqtt_uri << std::endl;

        // 2. CAMADA DE DADOS
        auto db = std::make_shared<DatabaseDriver>(db_path);

        // 3. CAMADA DE SERVIÇOS
        auto session_mgr = std::make_shared<SessionManager>(db);
        auto alarm_eng = std::make_shared<AlarmEngine>();
        auto supervisor = std::make_shared<InfusionSupervisor>(session_mgr, alarm_eng);

        // 4. CAMADA DE COMUNICAÇÃO (Usando config)
        MqttClient mqtt(mqtt_uri, mqtt_id);

        mqtt.set_on_message([supervisor](std::string topic, std::string payload) {
            if(topic == "bomba/status")
            {
                try
                {
                    json j = json::parse(payload);
                    Telemetry t = Telemetry::from_json(j);
                    supervisor->on_telemetry_received(t);
                }
                catch(...)
                {}
            }
        });

        // 5. API WEB
        // [ALTERADO] Passamos 'db_path' para o servidor configurar a rota de histórico
        HttpServer server(supervisor, &mqtt, db_path);

        std::thread web_thread([&server, web_port]() { server.run(web_port); });

        // 6. INÍCIO
        mqtt.start();

        std::cout << "[SYSTEM] Monitoramento Iniciado." << std::endl;

        while(running)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        server.stop();
        if(web_thread.joinable())
            web_thread.join();
        mqtt.stop();
    }
    catch(const std::exception& e)
    {
        std::cerr << "[CRITICAL] " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
