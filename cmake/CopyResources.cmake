#set(IN_RESOURCE_DIR "${CMAKE_SOURCE_DIR}/res")
#set(OUT_RESOURCE_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res")
#
#add_custom_target(copy_resources ALL
#        COMMAND ${CMAKE_COMMAND} -E rm -rf "${OUT_RESOURCE_DIR}"
#        COMMAND ${CMAKE_COMMAND} -E copy_directory "${IN_RESOURCE_DIR}" "${OUT_RESOURCE_DIR}"
#        COMMENT "Copying resources..."
#)

function(copy_resources_for_target target_name)
    set(resource_source_dir "${CMAKE_SOURCE_DIR}/res")

    add_custom_command(
            TARGET ${target_name}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E rm -rf "$<TARGET_FILE_DIR:${target_name}>/res"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${resource_source_dir}" "$<TARGET_FILE_DIR:${target_name}>/res"
            COMMENT "Copying resources for ${target_name}..."
    )
endfunction()