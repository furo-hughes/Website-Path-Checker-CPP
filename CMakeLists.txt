cmake_minimum_required(VERSION 3.20)
project(WebsitePathChecker VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL REQUIRED)

add_library(wpc_core
    src/config.cpp
    src/path_loader.cpp
    src/http_client.cpp
    src/checker.cpp
    src/console_reporter.cpp
    src/runner.cpp
    src/cli.cpp
)
target_include_directories(wpc_core PUBLIC include)
target_link_libraries(wpc_core PUBLIC CURL::libcurl)
target_compile_features(wpc_core PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(wpc_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(wpc_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()

add_executable(website-path-checker src/main.cpp)
target_link_libraries(website-path-checker PRIVATE wpc_core)

option(WPC_BUILD_TESTS "Build unit tests" ON)
if(WPC_BUILD_TESTS)
    enable_testing()
    add_executable(wpc_tests tests/unit_tests.cpp)
    target_link_libraries(wpc_tests PRIVATE wpc_core)
    add_test(NAME wpc_tests COMMAND wpc_tests)
endif()
