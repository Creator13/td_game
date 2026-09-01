add_executable(td_game game.cpp)

target_link_libraries(td_game
        PRIVATE engine
)

include(${CMAKE_SOURCE_DIR}/cmake/CopyResources.cmake)
copy_resources_for_target(td_game)