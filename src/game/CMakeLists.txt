add_executable(td_game main.cpp)

if (MSVC)
    target_link_options(td_game PRIVATE
            "$<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>"
            "$<$<CONFIG:Release>:/ENTRY:mainCRTStartup>"
    )
endif()

target_link_libraries(td_game
        PRIVATE engine
)