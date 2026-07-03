file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(example_number_stream
        ${SOURCES}
)

target_include_directories(example_number_stream PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(example_number_stream PRIVATE
        OdyPlanV1
)