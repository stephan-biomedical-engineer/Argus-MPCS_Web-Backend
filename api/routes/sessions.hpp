#pragma once

#include "crow.h"
#include <sqlite3.h>
#include <vector>
#include <string>
#include "crow/json.h"

// Função que registra a rota de histórico
// Recebe o caminho do banco de dados (ex: "/app/build/infusion.db")
inline void setup_session_routes(crow::SimpleApp& app, const std::string& db_path)
{
    CROW_ROUTE(app, "/api/sessions").methods(crow::HTTPMethod::GET)([db_path]() {
        sqlite3* db;
        // Abre o banco de dados (readonly para ser seguro)
        int rc = sqlite3_open_v2(db_path.c_str(), &db, SQLITE_OPEN_READONLY, NULL);

        if(rc)
        {
            std::string err_msg = sqlite3_errmsg(db);
            sqlite3_close(db);
            return crow::response(500, "Erro ao abrir DB: " + err_msg);
        }

        // Busca as últimas 50 sessões, da mais recente para a mais antiga
        const char* sql =
            "SELECT id, start_time, end_time, total_volume, status FROM sessions ORDER BY id DESC LIMIT 50;";
        sqlite3_stmt* stmt;

        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

        if(rc != SQLITE_OK)
        {
            std::string err_msg = sqlite3_errmsg(db);
            sqlite3_close(db);
            return crow::response(500, "Erro SQL: " + err_msg);
        }

        std::vector<crow::json::wvalue> sessions_list;

        // Itera sobre as linhas do banco
        while(sqlite3_step(stmt) == SQLITE_ROW)
        {
            crow::json::wvalue session;
            session["id"] = sqlite3_column_int(stmt, 0);

            const char* start = (const char*) sqlite3_column_text(stmt, 1);
            session["start_time"] = start ? start : "";

            const char* end = (const char*) sqlite3_column_text(stmt, 2);
            // Se end_time for NULL, significa que a infusão ainda está rodando ou parou abruptamente
            session["end_time"] = end ? end : "-";

            session["total_volume"] = sqlite3_column_double(stmt, 3);

            const char* status = (const char*) sqlite3_column_text(stmt, 4);
            session["status"] = status ? status : "UNKNOWN";

            sessions_list.push_back(session);
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);

        // O Crow converte o vector automaticamente para JSON Array
        return crow::response(crow::json::wvalue(sessions_list));
    });
}
