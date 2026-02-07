#include "mqtt_client.hpp"
#include <thread>
#include <iostream>

MqttClient::MqttClient(const std::string& server_address, const std::string& client_id)
    : client_(server_address, client_id)
{
    // setenv("MQTT_C_CLIENT_TRACE", "ON", 1);
    // setenv("MQTT_C_CLIENT_TRACE_LEVEL", "ERROR", 1);

    // Configurações de conexão
    conn_opts_.set_keep_alive_interval(20);
    conn_opts_.set_clean_session(true);

    ssl_opts_.set_trust_store("certs/ca.crt");          // CA da Raspberry
    ssl_opts_.set_key_store("certs/backend.crt");      // Certificado do backend
    ssl_opts_.set_private_key("certs/backend.key");    // Chave privada
    ssl_opts_.set_enable_server_cert_auth(true);

    conn_opts_.set_ssl(ssl_opts_);

    // Instala os callbacks nesta própria classe
    client_.set_callback(*this);
}

MqttClient::~MqttClient()
{
    disconnect();
}

void MqttClient::start()
{
    running_ = true;

    reconnect_thread_ = std::thread([this]() {
        while(running_)
        {
            if(!client_.is_connected())
            {
                try
                {
                    std::cout << "[MQTT] Tentando reconectar..." << std::endl;
                    client_.connect(conn_opts_, nullptr, *this);
                }
                catch(const mqtt::exception& e)
                {
                    std::cerr << "[MQTT] Exception: " << e.what()
                            << " | Reason code: " << e.get_reason_code()
                            << std::endl;
                }
            }

            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    });
}

void MqttClient::stop()
{
    running_ = false;

    if(reconnect_thread_.joinable())
        reconnect_thread_.join();

    disconnect();
}

void MqttClient::connect()
{
    try
    {
        std::cout << "[MQTT] Tentando conectar em " << client_.get_server_uri() << "..." << std::endl;
        // Conecta de forma assíncrona, usando 'this' para ouvir o sucesso/falha
        client_.connect(conn_opts_, nullptr, *this);
    }
    catch(const std::exception& e)
    {
        std::cerr << "[MQTT] std::exception: " << e.what() << std::endl;
    }
}

void MqttClient::disconnect()
{
    if(client_.is_connected())
    {
        try
        {
            client_.disconnect()->wait();
            std::cout << "[MQTT] Desconectado." << std::endl;
        }
        catch(...)
        {
            std::cerr << "[MQTT] Erro desconhecido na conexão" << std::endl;
        }
    }
}

void MqttClient::subscribe(const std::string& topic)
{
    if(!client_.is_connected())
    {
        std::cerr << "[MQTT] Erro: Tentativa de subscribe desconectado." << std::endl;
        return;
    }
    client_.subscribe(topic, 1); // QoS 1
    std::cout << "[MQTT] Inscrito no tópico: " << topic << std::endl;
}

void MqttClient::publish(const std::string& topic, const std::string& payload)
{
    if(!client_.is_connected())
        return;

    try
    {
        client_.publish(topic, payload.c_str(), payload.size(), 1, false);
        // std::cout << "[MQTT] Enviado para " << topic << std::endl; // Verboso demais, descomente se quiser
    }
    catch(const std::exception& e)
    {
        std::cerr << "[MQTT] Erro ao publicar: " << e.what() << std::endl;
    }
}

void MqttClient::set_on_message(std::function<void(std::string, std::string)> cb)
{
    on_message_cb_ = cb;
}

// --- Eventos do Paho ---

void MqttClient::on_success(const mqtt::token&)
{
    std::cout << "[MQTT] Conectado." << std::endl;
    subscribe("bomba/status");
}

void MqttClient::on_failure(const mqtt::token& tok)
{
    std::cerr << "[MQTT] Falha na conexão";

    if(tok.get_return_code() != 0)
        std::cerr << " | Return code: " << tok.get_return_code();

    std::cerr << std::endl;
}

void MqttClient::connection_lost(const std::string& cause)
{
    std::cerr << "[MQTT] Conexão perdida! Causa: " << cause << std::endl;
    // Aqui você poderia colocar uma lógica de reconnect automático
}

void MqttClient::message_arrived(mqtt::const_message_ptr msg)
{
    if(on_message_cb_)
    {
        // Repassa para a camada de cima (Main ou Service)
        on_message_cb_(msg->get_topic(), msg->to_string());
    }
}

void MqttClient::delivery_complete(mqtt::delivery_token_ptr token)
{
    // Confirmação que a mensagem saiu do computador
}
