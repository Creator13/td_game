file(GLOB_RECURSE ENGINE_SOURCES "*.cpp" "*.h")

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_library(engine STATIC ${ENGINE_SOURCES})

target_link_libraries(engine
        PUBLIC math
        PRIVATE spdlog::spdlog
        PRIVATE glfw
        PRIVATE glad::glad
)

include(${CMAKE_SOURCE_DIR}/cmake/CopyResources.cmake)
add_dependencies(engine copy_resources)

target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})