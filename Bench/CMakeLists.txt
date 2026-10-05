find_package(benchmark CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)

file(GLOB JUGX_BENCH_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# CMake generates its own PCH translation unit; pch.cpp would be a duplicate.
list(REMOVE_ITEM JUGX_BENCH_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/pch.cpp")

add_executable(JugXBench ${JUGX_BENCH_SOURCES})

target_precompile_headers(JugXBench PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/pch.h")

target_link_libraries(JugXBench PRIVATE
    JugX::JugX
    benchmark::benchmark
    nlohmann_json::nlohmann_json
)

if(TARGET rapidjson)
    target_link_libraries(JugXBench PRIVATE rapidjson)
else()
    target_include_directories(JugXBench PRIVATE ${RAPIDJSON_INCLUDE_DIRS})
endif()
