cmake_minimum_required(VERSION 3.20)

project(LittleHarvest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

add_executable(LittleHarvest
    src/main.cpp
    src/FarmPlot.cpp
)

target_link_libraries(LittleHarvest
    PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)
