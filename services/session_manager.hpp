#pragma once
#include <memory>
#include "../drivers/database_driver.hpp"
#include "../models/telemetry.hpp"

class SessionManager
{
private:
    std::shared_ptr<DatabaseDriver> db_;
    int current_session_id_;
    bool is_infusing_;

public:
    SessionManager(std::shared_ptr<DatabaseDriver> db);

    void process_telemetry(const Telemetry& t);
    int get_current_session_id() const
    {
        return current_session_id_;
    }
};
