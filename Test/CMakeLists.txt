find_package(GTest CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)

file(GLOB JUGX_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(JugXTest ${JUGX_TEST_SOURCES})

target_link_libraries(JugXTest PRIVATE
    JugX::JugX
    GTest::gtest
    nlohmann_json::nlohmann_json
)

# RapidJSON ships no imported target in every layout; fall back to includes.
if(TARGET rapidjson)
    target_link_libraries(JugXTest PRIVATE rapidjson)
else()
    target_include_directories(JugXTest PRIVATE ${RAPIDJSON_INCLUDE_DIRS})
endif()

include(GoogleTest)
gtest_discover_tests(JugXTest)
