add_executable(td_game main.cpp)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

target_link_libraries(td_game
        PRIVATE engine_math glfw glad::glad
)