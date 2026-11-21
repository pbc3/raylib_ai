cmake_minimum_required(VERSION 3.22)

project(GameAI LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
add_executable(GameAI main.cpp arena.cpp utils.cpp Enemy.cpp player.cpp pathfinding.cpp entity.cpp bullet.cpp)
if(MINGW)
    target_link_options(GameAI PRIVATE -static)
endif()

add_custom_target(copy_arena
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:GameAI>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/arena.csv"
        "$<TARGET_FILE_DIR:GameAI>/assets/arena.csv"
    VERBATIM
)
add_dependencies(GameAI copy_arena)

add_subdirectory(vendor/raylib)

target_link_libraries(GameAI PRIVATE raylib)


add_custom_target(run
    COMMAND $<TARGET_FILE:GameAI>
    DEPENDS GameAI
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    USES_TERMINAL
)


