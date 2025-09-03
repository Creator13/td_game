find_package(Catch2 3 REQUIRED CONFIG)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS
        "math/*.cpp"
)

add_executable(tests ${TEST_SOURCES})

target_link_libraries(tests PRIVATE
        engine_math
        Catch2::Catch2WithMain)