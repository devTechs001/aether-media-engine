cmake_minimum_required(VERSION 3.16)
project(BasicPlayerExample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find AETHER
find_package(AetherMediaEngine REQUIRED CONFIG)

add_executable(basic-player main.cpp)
target_link_libraries(basic-player PRIVATE
    AetherMediaEngine::aether_core
)
