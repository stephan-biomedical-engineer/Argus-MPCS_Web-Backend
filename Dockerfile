# 1. Escolhemos a imagem base (Ubuntu completo para ter compatibilidade)
FROM ubuntu:24.04

# 2. Evita perguntas de fuso horário durante a instalação
ARG DEBIAN_FRONTEND=noninteractive

# 3. Instalamos as ferramentas de compilação e as bibliotecas do projeto
# (Note que estamos instalando o Paho, SQLite, CMake, SSL e JSON direto dos repositórios)
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    avahi-utils \
    libnss-mdns \
    openssh-client \
    libsqlite3-dev \
    libssl-dev \
    nlohmann-json3-dev \
    libasio-dev \
    libboost-all-dev \
    && rm -rf /var/lib/apt/lists/*
    
# 4. Instalação manual do Paho MQTT (O C++ wrapper é chato de achar no apt as vezes)
# Vamos compilar o Paho C e o Paho C++ direto no Docker para garantir
WORKDIR /tmp
RUN git clone https://github.com/eclipse/paho.mqtt.c.git && \
    cd paho.mqtt.c && \
    cmake -Bbuild -H. -DPAHO_ENABLE_TESTING=OFF -DPAHO_BUILD_STATIC=ON \
    -DPAHO_WITH_SSL=ON -DPAHO_HIGH_PERFORMANCE=ON && \
    cmake --build build/ --target install && \
    ldconfig

RUN git clone https://github.com/eclipse/paho.mqtt.cpp.git && \
    cd paho.mqtt.cpp && \
    cmake -Bbuild -H. -DPAHO_BUILD_STATIC=ON -DPAHO_BUILD_DOCUMENTATION=FALSE && \
    cmake --build build/ --target install && \
    ldconfig

RUN sed -i 's/hosts:          files dns/hosts:          files mdns4_minimal [NOTFOUND=return] dns mdns4/' /etc/nsswitch.conf

# 5. Configura o diretório de trabalho da nossa aplicação
WORKDIR /app

RUN mkdir -p /root/.ssh && chmod 700 /root/.ssh

# 6. Copia todo o seu código para dentro do Docker
COPY . .

# 7. Prepara a pasta build e compila o projeto
WORKDIR /app/build
RUN cmake .. && make -j$(nproc)

# 8. Expõe a porta 8080 (Web)
EXPOSE 8080

# 9. Comando para rodar quando o container iniciar
CMD ["./InfusionBackend"]
