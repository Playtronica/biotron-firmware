file(GLOB src "*.c")
target_sources(${TARGET_NAME} PUBLIC ${src})
target_include_directories(${TARGET_NAME} PUBLIC ../include)