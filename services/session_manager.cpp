#include "session_manager.hpp"
#include <iostream>

SessionManager::SessionManager(std::shared_ptr<DatabaseDriver> db)
    : db_(db), current_session_id_(-1), is_infusing_(false)
{}

void SessionManager::process_telemetry(const Telemetry& t)
{
    bool device_running = (t.state == "RUNNING" || t.state == "BOLUS");

    // Borda de subida: Começou a rodar
    if(!is_infusing_ && device_running)
    {
        current_session_id_ = db_->create_session();
        is_infusing_ = true;
        std::cout << "[SESSION] Nova sessao iniciada: " << current_session_id_ << std::endl;
    }
    // Borda de descida: Parou
    else if(is_infusing_ && !device_running)
    {
        if(current_session_id_ != -1)
        {
            db_->end_session(current_session_id_, t.volume_infused_ml);
            std::cout << "[SESSION] Sessao finalizada." << std::endl;
        }
        current_session_id_ = -1;
        is_infusing_ = false;
    }

    // Salva no banco (agora passando o ID da sessão corretamente)
    db_->save_telemetry(t, current_session_id_);
}
