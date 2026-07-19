file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(example_count_window
        ${SOURCES}
)

target_include_directories(example_count_window PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

if(ENABLE_LOOPED_SOURCE)
    target_compile_definitions(example_count_window PRIVATE
            ENABLE_LOOPED_SOURCE
    )
endif ()

target_link_libraries(example_count_window PRIVATE
        OdyPlanV2
)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/data/read.json
        ${CMAKE_CURRENT_BINARY_DIR}/read.json
        COPYONLY
)