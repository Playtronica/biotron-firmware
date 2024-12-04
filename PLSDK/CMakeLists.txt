cmake_minimum_required(VERSION 3.13)
project(PLSDK
        VERSION 2.0.0
        DESCRIPTION "Abstraction library for Playtronica projects"
        LANGUAGES C CXX ASM
)


add_library(PLSDK_lib STATIC
        src/cap_buttons.c
        src/commands.c
        src/PLSDK.c
        src/music.c
        src/usb_descriptors.c
)


target_link_libraries(PLSDK_lib PUBLIC pico_stdlib tinyusb_device)
target_include_directories(PLSDK_lib PUBLIC include)