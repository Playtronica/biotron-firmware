FROM ubuntu:22.04

WORKDIR /app
ENV PICO_SDK_PATH=/app/pico-sdk

# SETUP DEPENDENCES
RUN sh -xec 'apt update \
    && apt install -qqy --no-install-recommends git wget ca-certificates \
    software-properties-common make build-essential python3-serial python3-pip \
    gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib cmake'

# SETUP PICO SDK
RUN sh -xec 'git clone "https://github.com/raspberrypi/pico-sdk.git" && \
             cd pico-sdk && \
             git submodule update --init'


RUN sh -xec 'git clone "https://github.com/raspberrypi/picotool.git" && \
    cd picotool && \
    mkdir "build" && \
    cd build && \
    cmake -DPICO_SDK_PATH=../../pico-sdk .. && \
    make && \
    make install'

