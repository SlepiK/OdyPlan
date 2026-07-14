file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(example_read_json
        ${SOURCES}
)

target_include_directories(example_read_json PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

if(ENABLE_LOOPED_SOURCE)
    target_compile_definitions(example_read_json PRIVATE
            ENABLE_LOOPED_SOURCE
    )
endif ()

target_link_libraries(example_read_json PRIVATE
        OdyPlanV2
)

add_custom_command(
        TARGET example_read_json POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/data/read.json
            $<TARGET_FILE_DIR:example_read_json>/read.json
        COMMENT "Copying read.json to output directory"
)