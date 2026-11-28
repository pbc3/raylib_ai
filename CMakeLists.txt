cmake_minimum_required(VERSION 3.22)

project(GameAI LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
add_executable(GameAI main.cpp arena.cpp utils.cpp Enemy.cpp player.cpp pathfinding.cpp entity.cpp bullet.cpp fsm.cpp artefacts.cpp renderer.cpp game_level.cpp
    input.cpp)
if(MINGW)
    target_link_options(GameAI PRIVATE -static)
endif()


add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:GameAI>/assets"
    VERBATIM
)

add_dependencies(GameAI copy_assets)


add_subdirectory(vendor/raylib)

target_link_libraries(GameAI PRIVATE raylib)


add_custom_target(run
    COMMAND $<TARGET_FILE:GameAI>
    DEPENDS GameAI
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    USES_TERMINAL
)


