FROM ubuntu:18.04

ARG DEBIAN_FRONTEND=noninteractive
ADD usr /usr
ENV PICO_SDK_PATH=/usr/local/src/pico/pico-sdk
ENV PICO_TINYUSB_PATH=/usr/local/src/pico/pico-sdk/lib/tinyusb
RUN sh -xec 'apt update \
    && apt install -qqy --no-install-recommends git wget ca-certificates \
    software-properties-common make build-essential python3-serial python3-pip \
    gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib'
RUN sh -xec 'wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null'
RUN sh -xec 'apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main" '
RUN sh -xec 'apt update && apt install -y cmake'

RUN mkdir -p /build