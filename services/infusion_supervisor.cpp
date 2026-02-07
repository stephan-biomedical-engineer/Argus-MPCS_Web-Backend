#include "infusion_supervisor.hpp"
#include <iostream>

InfusionSupervisor::InfusionSupervisor(std::shared_ptr<SessionManager> sm, std::shared_ptr<AlarmEngine> ae)
    : session_manager_(sm), alarm_engine_(ae), running_(true)
{

    current_state_.state = "OFFLINE";
    last_update_time_ = std::chrono::system_clock::now();

    // Inicia thread
    watchdog_thread_ = std::thread(&InfusionSupervisor::watchdog_loop, this);
}

InfusionSupervisor::~InfusionSupervisor()
{
    running_ = false;
    if(watchdog_thread_.joinable())
        watchdog_thread_.join();
}

void InfusionSupervisor::watchdog_loop()
{
    while(running_)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::lock_guard<std::mutex> lock(mtx_);
        auto now = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_update_time_).count();

        // Se passar 5 segundos sem dados, marca como OFFLINE
        if(elapsed > 5 && current_state_.state != "OFFLINE")
        {
            std::cout << "[WATCHDOG] Bomba parou de responder." << std::endl;
            current_state_.state = "OFFLINE";
            current_state_.real_rate_ml_h = 0;
            current_state_.alarm_active = true;
        }
    }
}

void InfusionSupervisor::on_telemetry_received(const Telemetry& t)
{
    std::lock_guard<std::mutex> lock(mtx_);

    // 1. Atualiza memória RAM (para API ver rápido)
    current_state_ = t;
    last_update_time_ = std::chrono::system_clock::now();

    // 2. Verifica Alarmes
    auto anomalies = alarm_engine_->check_anomalies(t);
    if(!anomalies.empty())
    {
        current_state_.state = "ALARM";
        std::cout << "[ALARM ENGINE] Anomalia detectada: " << anomalies[0].message << std::endl;
    }

    // 3. Processa Sessão e Banco
    // Note que não precisamos travar mutex aqui se o session_manager for thread-safe (ele é via SQLite lock)
    // Mas para garantir, soltamos o lock antes de operações pesadas de IO se possível.
    // Para simplificar agora, faremos direto.
    session_manager_->process_telemetry(current_state_);
}

Telemetry InfusionSupervisor::get_latest_telemetry()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return current_state_;
}
