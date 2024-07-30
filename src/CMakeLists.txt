file(GLOB Biotron_SRC "*.c")
target_sources(${TARGET_NAME} PUBLIC ${Biotron_SRC})
target_include_directories(${TARGET_NAME} PUBLIC ../include)