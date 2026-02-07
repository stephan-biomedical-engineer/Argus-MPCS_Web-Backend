#pragma once
#include <string>
#include <chrono>

struct Session
{
    long long id;          // ID no Banco de Dados
    long long start_time;  // Unix Timestamp
    long long end_time;    // Unix Timestamp (0 se ainda ativa)
    double total_volume;   // Volume acumulado na sessão
    std::string drug_name; // (Opcional) Nome do medicamento
    bool is_active;
};
