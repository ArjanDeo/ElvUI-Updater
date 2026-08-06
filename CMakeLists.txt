cmake_minimum_required(VERSION 3.20)

project(ElvUI-Updater)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json CONFIG REQUIRED)

file(GLOB_RECURSE SOURCES
    src/*.cpp
)

add_executable(ElvUI-Updater
    ${SOURCES}
)

target_link_libraries(ElvUI-Updater
    PRIVATE
    nlohmann_json::nlohmann_json
)