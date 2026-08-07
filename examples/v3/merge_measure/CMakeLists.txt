file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

set(TARGET_NAME odyplan_example_merge_measure)

add_executable(${TARGET_NAME}
        ${SOURCES}
)

target_include_directories(${TARGET_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

if(ENABLE_LOOPED_SOURCE)
    target_compile_definitions(${TARGET_NAME} PRIVATE
            ENABLE_LOOPED_SOURCE
    )
endif ()

target_link_libraries(${TARGET_NAME} PRIVATE
        odyplan::core
        odyplan::basic
)

enable_clang_tidy_on_target(${TARGET_NAME})

file(COPY
        ${CMAKE_CURRENT_SOURCE_DIR}/data/read.ndjson
        ${CMAKE_CURRENT_SOURCE_DIR}/data/read2.ndjson
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

if(ENABLE_TOOLING)
    register_lint_sources(${SOURCES} ${HEADERS})
    register_tidy_sources(${SOURCES})
endif()