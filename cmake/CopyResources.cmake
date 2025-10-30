set(IN_RESOURCE_DIR "${CMAKE_SOURCE_DIR}/res")
set(OUT_RESOURCE_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res")

add_custom_target(copy_resources ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${IN_RESOURCE_DIR} ${OUT_RESOURCE_DIR}
        COMMENT "Copying resources..."
)
