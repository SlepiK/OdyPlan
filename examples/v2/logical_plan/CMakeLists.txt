file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(example_logical_plan
        ${SOURCES}
)

target_include_directories(example_logical_plan PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(example_logical_plan PRIVATE
        OdyPlanV2
)