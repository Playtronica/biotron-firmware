# Fetch all files in current dir
file(GLOB FILES *.c *.h)
add_library(PLSDK_library ${FILES})


if(TEST_BUILD)
    # pass -D TEST_BUILD when compiling
    target_compile_definitions(PLSDK_library PUBLIC -D TEST_BUILD)
    target_link_libraries(PLSDK_library PUBLIC pico_stdlib tinyusb_device)
else()
    # normal build, link with pico_stdlib
    target_link_libraries(PLSDK_library PUBLIC pico_stdlib tinyusb_device)
    
endif(TEST_BUILD)


# IMPORTANT define where to find the files when the lib is included from a different folder!
target_include_directories(PLSDK_library PUBLIC ../include)