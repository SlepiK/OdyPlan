file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(example_select
        ${SOURCES}
)

target_include_directories(example_select PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

if(ENABLE_LOOPED_SOURCE)
    target_compile_definitions(example_select PRIVATE
            ENABLE_LOOPED_SOURCE
    )
endif ()

target_link_libraries(example_select PRIVATE
        OdyPlanV2
)

enable_clang_tidy_on_target(example_select)

if(ENABLE_TOOLING)
    register_lint_sources(${SOURCES})
endif()