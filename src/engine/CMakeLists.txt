include(FetchContent)

# =============================
#      engine core target
# =============================

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
        "assets/*.cpp" "assets/*.h" "assets/*.inl"
        "core/*.cpp" "core/*.h" "core/*.inl"
        "rendering/*.cpp" "rendering/*.h" "rendering/*.inl"
        "util/*.cpp" "util/*.h" "util/*.inl"
        "datatype.h" "datatype.cpp"
)

## Vcpkg packages
find_package(glfw3 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(xxHash CONFIG REQUIRED)
find_package(fastgltf CONFIG REQUIRED)
find_package(flecs CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(cpptrace CONFIG REQUIRED)
find_package(msdfgen CONFIG REQUIRED)
find_package(Freetype REQUIRED)
find_package(simdutf CONFIG REQUIRED)

## Tracy
FetchContent_Declare(
        tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
        GIT_TAG v0.13.1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
set(TRACY_ON_DEMAND ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(tracy)

## msdf-atlas-gen
FetchContent_Declare(
        msdf_atlas_gen
        GIT_REPOSITORY https://github.com/Chlumsky/msdf-atlas-gen.git
        GIT_TAG        v1.4
)
set(MSDF_ATLAS_BUILD_STANDALONE OFF CACHE BOOL "Disable the standalone executable" FORCE)
set(MSDF_ATLAS_USE_SKIA OFF CACHE BOOL "Disable the Skia library dependency" FORCE)
set(MSDF_ATLAS_NO_ARTERY_FONT ON CACHE BOOL "Disable Artery Font export" FORCE)
set(MSDF_ATLAS_MSDFGEN_EXTERNAL ON CACHE BOOL "Use external msdfgen from vcpkg" FORCE)
set(MSDF_ATLAS_DYNAMIC_RUNTIME ON CACHE BOOL "Force dynamic MSVC runtime (/MD) to match vcpkg" FORCE)

FetchContent_MakeAvailable(msdf_atlas_gen)

target_include_directories(msdf-atlas-gen SYSTEM INTERFACE
        "${msdf_atlas_gen_SOURCE_DIR}"
)
if(MSVC)
    set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "/external:I ")
endif ()

## Declare engine_core library
add_library(engine_core STATIC ${ENGINE_SOURCES})

target_link_libraries(engine_core
        PUBLIC math
        PUBLIC spdlog::spdlog
        PUBLIC $<IF:$<TARGET_EXISTS:flecs::flecs>,flecs::flecs,flecs::flecs_static>
        PUBLIC TracyClient
        PRIVATE glad
        PRIVATE glfw
        PRIVATE xxHash::xxhash
        PRIVATE fastgltf::fastgltf
        PRIVATE stb_image
        PRIVATE magic_enum::magic_enum
        PRIVATE misc_headers
        PRIVATE cpptrace::cpptrace
        PRIVATE Freetype::Freetype
        PRIVATE msdf-atlas-gen::msdf-atlas-gen
        PRIVATE simdutf::simdutf
)

## Copy engine resources to bin directory
include(${CMAKE_SOURCE_DIR}/cmake/CopyResources.cmake)
add_dependencies(engine_core copy_resources)

## Spdlog debug level depends on target
target_compile_definitions(engine_core PUBLIC
        $<$<CONFIG:RelWithDebInfo>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE>
        $<$<CONFIG:Release>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO>
        $<$<CONFIG:Debug>:SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE>
)

## Set DEBUG_BUILD definition on Debug and RelWithDebInfo targets
target_compile_definitions(engine_core PUBLIC
        $<$<CONFIG:Debug>:DEBUG_BUILD>
        $<$<CONFIG:RelWithDebInfo>:DEBUG_BUILD>
        $<$<CONFIG:Release>:RELEASE>
)

## Flecs config
target_compile_definitions(engine_core PUBLIC
        FLECS_NO_APP
)

target_include_directories(engine_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_library(engine::core ALIAS engine_core)

# =============================
#   engine entry point target
# =============================

add_library(engine STATIC engine.h engine.cpp)

if (MSVC)
    target_link_options(engine PRIVATE
            "$<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>"
            "$<$<CONFIG:Release>:/ENTRY:mainCRTStartup>"
    )
endif ()

target_link_libraries(engine PUBLIC engine::core)

