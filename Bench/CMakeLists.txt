find_package(benchmark CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)

file(GLOB JUGX_BENCH_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(JugXBench ${JUGX_BENCH_SOURCES})

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
