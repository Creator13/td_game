file(GLOB_RECURSE ENGINE_SOURCES "*.cpp" "*.h")

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(xxHash CONFIG REQUIRED)
find_package(fastgltf CONFIG REQUIRED)
find_package(flecs CONFIG REQUIRED)

add_library(engine STATIC ${ENGINE_SOURCES})

target_link_libraries(engine
        PUBLIC math
        PUBLIC spdlog::spdlog
        PUBLIC $<IF:$<TARGET_EXISTS:flecs::flecs>,flecs::flecs,flecs::flecs_static>
        PRIVATE glad::glad
        PRIVATE glfw
        PRIVATE xxHash::xxhash
        PRIVATE fastgltf::fastgltf
)

include(${CMAKE_SOURCE_DIR}/cmake/CopyResources.cmake)
add_dependencies(engine copy_resources)

target_compile_definitions(engine PUBLIC
        $<$<CONFIG:Debug>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG>
        $<$<CONFIG:Release>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO>
)

target_compile_definitions(engine PUBLIC
        $<$<CONFIG:Debug>:DEBUG_BUILD>
        $<$<CONFIG:RelWithDebInfo>:DEBUG_BUILD>
        $<$<CONFIG:Release>:RELEASE>
)

target_compile_definitions(engine PUBLIC
        FLECS_NO_APP
        FLECS_
)

target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})