find_package(Catch2 3 REQUIRED CONFIG)

file(GLOB CORE_TEST_SOURCES CONFIGURE_DEPENDS
        "*.cpp"
)

add_executable(core_tests ${CORE_TEST_SOURCES})

target_link_libraries(core_tests PRIVATE
        engine
        Catch2::Catch2WithMain)