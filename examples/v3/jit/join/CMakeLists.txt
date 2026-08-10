file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

set(TARGET_NAME odyplan_example_join)

add_executable(${TARGET_NAME}
        ${SOURCES}
)

target_include_directories(${TARGET_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_definitions(${TARGET_NAME} PRIVATE
        #SIERRA_USER_PROGRAM_CLOCK
)

target_link_libraries(${TARGET_NAME} PRIVATE
        odyplan::core
        odyplan::basic
)

odyplan_link_llvm_jit(${TARGET_NAME} PRIVATE)

enable_clang_tidy_on_target(${TARGET_NAME})

file(COPY
        data/left.ndjson
        data/right.ndjson
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

if(ENABLE_TOOLING)
    register_lint_sources(${SOURCES} ${HEADERS})
    register_tidy_sources(${SOURCES})
endif()