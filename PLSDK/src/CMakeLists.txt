file(GLOB PLSDK_SRC "*.c")

target_sources(${TARGET_NAME} PUBLIC ${PLSDK_SRC})
target_include_directories(${TARGET_NAME} PUBLIC ../include)