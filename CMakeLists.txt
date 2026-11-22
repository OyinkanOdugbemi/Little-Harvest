cmake_minimum_required(VERSION 3.22)

project(LittleHarvest VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Use an installed SFML 3 if there is one (e.g. MSYS2: pacman -S mingw-w64-ucrt-x86_64-sfml),
# otherwise download and build it automatically.
find_package(SFML 3 COMPONENTS Graphics Window System Audio QUIET)

if(NOT SFML_FOUND)
    message(STATUS "SFML 3 not found - fetching it from GitHub (this takes a minute the first time)")
    include(FetchContent)
    FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.2
        GIT_SHALLOW ON)
    set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(SFML)
endif()

add_executable(LittleHarvest
    src/main.cpp
    src/Game.cpp
    src/FarmPlot.cpp
    src/Crop.cpp
    src/Art.cpp
    src/Ui.cpp
    src/Effects.cpp
    src/Scenery.cpp
    src/SoundBank.cpp
)

target_link_libraries(LittleHarvest
    PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
)

if(MSVC)
    target_compile_options(LittleHarvest PRIVATE /W4)
else()
    target_compile_options(LittleHarvest PRIVATE -Wall -Wextra)
endif()

# No console window behind the game on Windows release builds.
if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set_target_properties(LittleHarvest PROPERTIES WIN32_EXECUTABLE ON)
    if(MSVC)
        target_link_options(LittleHarvest PRIVATE /ENTRY:mainCRTStartup)
    endif()
endif()

# Copy the fonts next to the executable so it runs straight from the build folder.
add_custom_command(TARGET LittleHarvest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:LittleHarvest>/assets)
