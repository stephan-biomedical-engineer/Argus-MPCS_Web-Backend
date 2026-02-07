#pragma once

#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <memory>
#include "../models/telemetry.hpp"
#include "session_manager.hpp"
#include "alarm_engine.hpp"

class InfusionSupervisor
{
private:
    Telemetry current_state_;
    std::mutex mtx_;

    // Dependências (Agora declaradas aqui!)
    std::shared_ptr<SessionManager> session_manager_;
    std::shared_ptr<AlarmEngine> alarm_engine_;

    // Watchdog
    std::atomic<bool> running_;
    std::thread watchdog_thread_;
    std::chrono::system_clock::time_point last_update_time_;

    void watchdog_loop(); // <--- A declaração que faltava

public:
    // Construtor atualizado recebendo dependências
    InfusionSupervisor(std::shared_ptr<SessionManager> sm, std::shared_ptr<AlarmEngine> ae);
    ~InfusionSupervisor();

    void on_telemetry_received(const Telemetry& t); // <--- A declaração que faltava
    Telemetry get_latest_telemetry();
};
