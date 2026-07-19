file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(example_project
        ${SOURCES}
)

target_include_directories(example_project PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

if(ENABLE_LOOPED_SOURCE)
    target_compile_definitions(example_project PRIVATE
            ENABLE_LOOPED_SOURCE
    )
endif ()

target_link_libraries(example_project PRIVATE
        OdyPlanV2
)

enable_clang_tidy_on_target(example_project)

if(ENABLE_TOOLING)
    register_lint_sources(${SOURCES})
endif()