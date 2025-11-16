file(GLOB_RECURSE ENGINE_SOURCES "*.cpp" "*.h")

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(xxHash CONFIG REQUIRED)

add_library(engine STATIC ${ENGINE_SOURCES})

target_link_libraries(engine
        PUBLIC math
        PRIVATE spdlog::spdlog
        PRIVATE glad::glad
        PRIVATE glfw
        PRIVATE xxHash::xxhash
)

include(${CMAKE_SOURCE_DIR}/cmake/CopyResources.cmake)
add_dependencies(engine copy_resources)

target_compile_definitions(engine PUBLIC
        $<$<CONFIG:Debug>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG>
        $<$<CONFIG:Release>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO>
)

target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})