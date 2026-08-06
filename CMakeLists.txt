cmake_minimum_required(VERSION 3.20)

project(ElvUI-Updater)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(nlohmann_json CONFIG REQUIRED)

file(GLOB_RECURSE SOURCES
    src/*.cpp
)

add_executable(ElvUI-Updater
    ${SOURCES}
)

target_include_directories(ElvUI-Updater
    PRIVATE
        include
)

target_link_libraries(ElvUI-Updater
    PRIVATE
    nlohmann_json::nlohmann_json
)