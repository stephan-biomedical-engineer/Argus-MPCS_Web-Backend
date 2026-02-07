#pragma once

#include <string>
#include <sqlite3.h>
#include "../models/telemetry.hpp"

class DatabaseDriver
{
private:
    sqlite3* db_;
    std::string db_path_;

public:
    // Apenas declarações (terminam com ;)
    DatabaseDriver(const std::string& path);
    ~DatabaseDriver();

    void init();

    // Novas funções de Sessão
    int create_session();
    void end_session(int session_id, double final_vol);

    // Assinatura atualizada: agora pede o session_id
    bool save_telemetry(const Telemetry& t, int session_id = -1);
};
