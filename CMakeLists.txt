cmake_minimum_required(VERSION 3.21)
project(music_player LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell CMake where the drag-and-dropped Slint lives
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/external/slint")
find_package(Slint REQUIRED)

add_executable(music_player
    src/main.cpp
    src/audio/miniaudioImplementation.cpp
)

# miniaudio (header only)
target_include_directories(music_player PRIVATE external/miniaudio)
if(UNIX AND NOT APPLE)
    target_link_libraries(music_player PRIVATE dl pthread m)
endif()

# Slint: compiles the .slint file and generates app.h
slint_target_sources(music_player ui/main.slint)
target_link_libraries(music_player PRIVATE Slint::Slint)

# Windows: copy slint_cpp.dll next to the exe
if(WIN32)
    add_custom_command(TARGET music_player POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_RUNTIME_DLLS:music_player> $<TARGET_FILE_DIR:music_player>
        COMMAND_EXPAND_LISTS)
endif()