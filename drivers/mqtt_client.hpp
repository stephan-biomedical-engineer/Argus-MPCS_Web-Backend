#pragma once

#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <mqtt/async_client.h>
#include <mqtt/ssl_options.h>


class MqttClient : public virtual mqtt::callback, public virtual mqtt::iaction_listener
{
private:
    mqtt::async_client client_;
    mqtt::connect_options conn_opts_;
    mqtt::ssl_options ssl_opts_;

    std::function<void(std::string, std::string)> on_message_cb_;

    std::atomic<bool> running_{false};
    std::thread reconnect_thread_;

    // eventos do Paho
    void connection_lost(const std::string& cause) override;
    void message_arrived(mqtt::const_message_ptr msg) override;
    void delivery_complete(mqtt::delivery_token_ptr token) override;

    void on_failure(const mqtt::token& tok) override;
    void on_success(const mqtt::token& tok) override;

    void connect();
    void disconnect();

public:
    MqttClient(const std::string& server_address, const std::string& client_id);
    virtual ~MqttClient();

    void start();
    void stop();

    void subscribe(const std::string& topic);
    void publish(const std::string& topic, const std::string& payload);

    void set_on_message(std::function<void(std::string, std::string)> cb);
};
