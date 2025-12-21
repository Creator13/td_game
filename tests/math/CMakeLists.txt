find_package(Catch2 3 REQUIRED CONFIG)

file(GLOB MATH_TEST_SOURCES CONFIGURE_DEPENDS
        "*.cpp"
)

add_executable(math_tests ${MATH_TEST_SOURCES})

target_link_libraries(math_tests PRIVATE
        math
        Catch2::Catch2WithMain)