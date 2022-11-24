cmake_minimum_required(VERSION 3.10)


set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
include(pico_sdk_import.cmake)

project(biotron C CXX ASM)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
        message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

if(DEFINED ENV{DEBUG_BUILD})
        message("Set debug build value '$ENV{DEBUG_BUILD}'")
        set(DEBUG_BUILD $ENV{DEBUG_BUILD})
else()
        set(DEBUG_BUILD "false")
endif()

if (${DEBUG_BUILD} MATCHES "layout")
        message(WARNING "There is set build for debug layout ATTENTION!!!")
        add_definitions(-DDEBUG_LAYOUT_BUILD)
        set(TARGET_NAME "biotron_layout")
else()
        message("Production build")
        set(TARGET_NAME "biotron")
endif()
add_definitions(-DPICO_XOSC_STARTUP_DELAY_MULTIPLIER=64)

pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

add_subdirectory(include)


file(GLOB biotron_SRC "src/*.c")

add_executable(${TARGET_NAME})
target_sources(${TARGET_NAME} PUBLIC main.c ${biotron_SRC})
target_include_directories(${TARGET_NAME} PUBLIC include())
pico_enable_stdio_usb(${TARGET_NAME} 0)
pico_enable_stdio_uart(${TARGET_NAME} 0)
target_link_libraries(${TARGET_NAME} pico_stdlib tinyusb_device)
pico_add_extra_outputs(${TARGET_NAME})

