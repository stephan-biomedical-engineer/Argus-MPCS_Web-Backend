cmake_minimum_required(VERSION 3.14)

# Correção do aviso de timestamp (Policy CMP0135)
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

project(InfusionBackend VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Dependências do Sistema ---
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

# --- Paho MQTT (CORREÇÃO DE SEGURANÇA) ---
# Removido "paho-mqtt3a" da lista.
# Agora exigimos EXPLICITAMENTE a versão com SSL ("s" no final).
find_library(PAHO_MQTT_C_LIB NAMES paho-mqtt3as REQUIRED)
find_library(PAHO_MQTT_CPP_LIB NAMES paho-mqttpp3 REQUIRED)

# --- Dependências via FetchContent (Baixa e Compila) ---
include(FetchContent)

# 1. JSON
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

# 2. Crow
FetchContent_Declare(
    Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(Crow)

# --- Configuração do Executável ---
file(GLOB_RECURSE SOURCES 
    "api/*.cpp" "drivers/*.cpp" "models/*.cpp" 
    "services/*.cpp" "system/*.cpp" "utl/*.cpp"
)

# --- CÓPIA AUTOMÁTICA DE RECURSOS ---
# Copia Config
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/system/config.json 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Copia Pasta Static (Frontend)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/static
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# [NOVO] Copia Pasta de Certificados
# Isso garante que ./certs/ca.crt exista ao lado do executável
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/certs
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(${PROJECT_NAME} ${SOURCES})

# Habilita SSL no Crow (caso use HTTPS no futuro)
target_compile_definitions(${PROJECT_NAME} PRIVATE CROW_ENABLE_SSL)

# Onde estão os arquivos .hpp
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/api
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers
    ${CMAKE_CURRENT_SOURCE_DIR}/models
    ${CMAKE_CURRENT_SOURCE_DIR}/services
    ${CMAKE_CURRENT_SOURCE_DIR}/utl
)

# Linkagem Final
target_link_libraries(${PROJECT_NAME} PRIVATE
    Boost::system Boost::thread SQLite::SQLite3
    ${PAHO_MQTT_CPP_LIB} ${PAHO_MQTT_C_LIB}
    nlohmann_json::nlohmann_json
    Crow::Crow
    pthread
    OpenSSL::SSL 
    OpenSSL::Crypto
)