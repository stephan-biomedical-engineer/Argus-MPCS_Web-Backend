#include "database_driver.hpp"
#include <iostream>
#include <chrono>

DatabaseDriver::DatabaseDriver(const std::string& path) : db_path_(path), db_(nullptr)
{
    init();
}

DatabaseDriver::~DatabaseDriver()
{
    if(db_)
    {
        sqlite3_close(db_);
        std::cout << "[DB] Conexão fechada." << std::endl;
    }
}

void DatabaseDriver::init()
{
    int rc = sqlite3_open(db_path_.c_str(), &db_);
    if(rc)
    {
        std::cerr << "[DB] Erro crítico ao abrir: " << sqlite3_errmsg(db_) << std::endl;
        return;
    }

    // Cria tabelas de SESSÕES e TELEMETRIA
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS sessions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            start_time INTEGER,
            end_time INTEGER,
            total_volume REAL,
            is_active INTEGER
        );
        CREATE TABLE IF NOT EXISTS telemetria (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            session_id INTEGER,
            timestamp INTEGER,
            state TEXT,
            volume_infused REAL,
            flow_rate REAL,
            alarm_active INTEGER,
            raw_json TEXT,
            FOREIGN KEY(session_id) REFERENCES sessions(id)
        );
    )";

    char* errMsg = 0;
    rc = sqlite3_exec(db_, sql, 0, 0, &errMsg);
    if(rc != SQLITE_OK)
    {
        std::cerr << "[DB] Erro init SQL: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    else
    {
        std::cout << "[DB] Tabelas verificadas." << std::endl;
    }
}

int DatabaseDriver::create_session()
{
    if(!db_)
        return -1;

    long long now = std::chrono::system_clock::now().time_since_epoch().count();
    // Cria uma sessão nova ativa
    std::string sql =
        "INSERT INTO sessions (start_time, total_volume, is_active) VALUES (" + std::to_string(now) + ", 0.0, 1);";

    char* errMsg = 0;
    if(sqlite3_exec(db_, sql.c_str(), 0, 0, &errMsg) != SQLITE_OK)
    {
        std::cerr << "[DB] Erro create_session: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return -1;
    }
    return (int) sqlite3_last_insert_rowid(db_);
}

void DatabaseDriver::end_session(int session_id, double final_vol)
{
    if(!db_ || session_id < 0)
        return;

    long long now = std::chrono::system_clock::now().time_since_epoch().count();

    // Atualiza data de fim e fecha a sessão
    std::string sql = "UPDATE sessions SET end_time=" + std::to_string(now) +
                      ", total_volume=" + std::to_string(final_vol) +
                      ", is_active=0 WHERE id=" + std::to_string(session_id) + ";";

    sqlite3_exec(db_, sql.c_str(), 0, 0, 0);
}

bool DatabaseDriver::save_telemetry(const Telemetry& t, int session_id)
{
    if(!db_)
        return false;

    // SQL atualizado com session_id
    std::string sql = "INSERT INTO telemetria (session_id, timestamp, state, volume_infused, flow_rate, alarm_active, "
                      "raw_json) VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;

    if(sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, 0) != SQLITE_OK)
    {
        std::cerr << "[DB] Erro prepare: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, session_id);
    sqlite3_bind_int64(stmt, 2, t.timestamp);
    sqlite3_bind_text(stmt, 3, t.state.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 4, t.volume_infused_ml);
    sqlite3_bind_double(stmt, 5, t.real_rate_ml_h);
    sqlite3_bind_int(stmt, 6, t.alarm_active ? 1 : 0);

    std::string json_str = t.to_json().dump();
    sqlite3_bind_text(stmt, 7, json_str.c_str(), -1, SQLITE_STATIC);

    if(sqlite3_step(stmt) != SQLITE_DONE)
    {
        std::cerr << "[DB] Erro step: " << sqlite3_errmsg(db_) << std::endl;
    }

    sqlite3_finalize(stmt);
    return true;
}
