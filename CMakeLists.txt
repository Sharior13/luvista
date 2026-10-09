cmake_minimum_required(VERSION 3.21)
project(music_player LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell CMake where the drag-and-dropped Slint lives
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/external/slint")
find_package(Slint REQUIRED)

# Our C++ code, plus the shine MP3 encoder (plain C files in external/shine)
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/shine/*.c"
)

# Check: how many shine .c files did CMake find? (should say 11)
file(GLOB SHINE_CHECK "${CMAKE_CURRENT_SOURCE_DIR}/external/shine/*.c")
list(LENGTH SHINE_CHECK SHINE_COUNT)
message(STATUS "shine .c files found: ${SHINE_COUNT} (should be 11)")

add_executable(music_player ${MY_SOURCES})

# miniaudio (header only) and shine (MP3 encoder)
target_include_directories(music_player PRIVATE external/miniaudio external/shine)
if(UNIX AND NOT APPLE)
    target_link_libraries(music_player PRIVATE dl pthread m)
endif()

# Slint: compiles the .slint file and generates app.h
slint_target_sources(music_player ui/app.slint)
target_link_libraries(music_player PRIVATE Slint::Slint)

# Windows: copy slint_cpp.dll next to the exe
if(WIN32)
    add_custom_command(TARGET music_player POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_RUNTIME_DLLS:music_player> $<TARGET_FILE_DIR:music_player>
        COMMAND_EXPAND_LISTS)
endif()