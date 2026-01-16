find_package(Catch2 3 REQUIRED CONFIG)

file(GLOB ENGINE_TEST_SOURCES CONFIGURE_DEPENDS
        "*.cpp"
)

add_executable(engine_tests ${ENGINE_TEST_SOURCES})

target_link_libraries(engine_tests PRIVATE
        engine::core
        Catch2::Catch2WithMain)