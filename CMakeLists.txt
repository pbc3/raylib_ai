cmake_minimum_required(VERSION 3.22)

project(GameAI LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(GameAI main.cpp arena.cpp utils.cpp)



add_subdirectory(vendor/raylib)

target_link_libraries(GameAI PRIVATE raylib)


add_custom_target(run
    COMMAND $<TARGET_FILE:GameAI>
    DEPENDS GameAI
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    USES_TERMINAL
)


