file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(example_full
        ${SOURCES}
)

target_include_directories(example_full PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

if(ENABLE_LOOPED_SOURCE)
    target_compile_definitions(example_full PRIVATE
            ENABLE_LOOPED_SOURCE
    )
endif ()

target_link_libraries(example_full PRIVATE
        OdyPlanV2
)

enable_clang_tidy_on_target(example_full)

if(ENABLE_TOOLING)
    register_lint_sources(${SOURCES})
endif()