cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
include(pico_sdk_import.cmake)

project(Biotron C CXX ASM)


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


IF(DEFINED ENV{FLASH_ID_STARTUP})
        message("$ENV{FLASH_ID_STARTUP}")
        add_definitions(-DFLASH_ID_STARTUP=$ENV{FLASH_ID_STARTUP})
ENDIF()

pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
)


add_executable(${TARGET_NAME})

add_subdirectory(src)
add_subdirectory(PLSDK)

target_sources(${TARGET_NAME} PUBLIC main.c)
target_include_directories(${TARGET_NAME} PUBLIC include())
target_link_libraries(${TARGET_NAME} pico_stdlib PLSDK_lib hardware_pwm hardware_adc hardware_flash)

pico_enable_stdio_usb(${TARGET_NAME} 1)
pico_enable_stdio_uart(${TARGET_NAME} 0)
pico_add_extra_outputs(${TARGET_NAME})