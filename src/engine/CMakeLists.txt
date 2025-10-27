file(GLOB_RECURSE ENGINE_SOURCES "*.cpp" "*.h")

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

add_library(engine STATIC ${ENGINE_SOURCES})

target_link_libraries(engine
        PUBLIC
        math
        PRIVATE
        glfw
        glad::glad
)

target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})