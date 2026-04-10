
add_library(math INTERFACE)

target_compile_definitions(math INTERFACE NOMINMAX WIN32_LEAN_AND_MEAN)
target_include_directories(math INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
