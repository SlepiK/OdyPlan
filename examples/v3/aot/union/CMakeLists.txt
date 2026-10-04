file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

# 1. Compiler

set(COMPILER_TARGET_NAME odyplan_example_aot_union_compiler)

add_executable(${COMPILER_TARGET_NAME}
        src/union_compiler.cpp
)

target_include_directories(${COMPILER_TARGET_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(${COMPILER_TARGET_NAME} PRIVATE
        odyplan::core
        odyplan::basic
)

odyplan_link_llvm_aot(${COMPILER_TARGET_NAME} PRIVATE)

# 2. Generate o.-File of Plan

set(GENERATED_PLAN_OBJECT ${CMAKE_CURRENT_BINARY_DIR}/union_plan.o)

add_custom_command(
        OUTPUT ${GENERATED_PLAN_OBJECT}
        COMMAND ${COMPILER_TARGET_NAME} ${GENERATED_PLAN_OBJECT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${COMPILER_TARGET_NAME}
        COMMENT "Compiling logical union-plan to native object file"
)

add_custom_target(odyplan_example_aot_union_plan DEPENDS ${GENERATED_PLAN_OBJECT})

#3. Final Step

set(TARGET_NAME odyplan_example_aot_union)

add_executable(${TARGET_NAME}
        src/odypipe.cpp
        ${GENERATED_PLAN_OBJECT}
)

target_include_directories(${TARGET_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

file(COPY
        data/nexmark.person.10k.2.ndjson
        data/nexmark.person.10k.ndjson
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

add_dependencies(${TARGET_NAME}
        odyplan_example_aot_union_plan
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

odyplan_link_llvm_aot(${TARGET_NAME} PRIVATE)

enable_clang_tidy_on_target(${TARGET_NAME})

if(ENABLE_TOOLING)
    register_lint_sources(${SOURCES} ${HEADERS})
    register_tidy_sources(${SOURCES})
endif()


#3. Check / Verify

set(REFERENCE_ODYSSEUS_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/data/ody_check_output.txt)
set(TEST_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/output.txt)

add_custom_target(odyplan_example_aot_union_run
        COMMAND $<TARGET_FILE:${TARGET_NAME}> > ${TEST_OUTPUT}
        DEPENDS ${TARGET_NAME}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running union executable"
)

add_custom_target(odyplan_example_aot_union_check
        COMMAND ${CMAKE_COMMAND} -E echo "=== Running union test ==="
        COMMAND ${CMAKE_COMMAND} -E compare_files
        --ignore-eol
        ${TEST_OUTPUT}
        ${REFERENCE_ODYSSEUS_OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E echo "=== Test PASSED ==="
        DEPENDS odyplan_example_aot_union_run ${REFERENCE_ODYSSEUS_OUTPUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Comparing union output against reference"
)

add_test(
        NAME odyplan_example_aot_union
        COMMAND ${CMAKE_COMMAND} -E compare_files
        --ignore-eol
        ${TEST_OUTPUT}
        ${REFERENCE_ODYSSEUS_OUTPUT}
)

set_tests_properties(odyplan_example_aot_union PROPERTIES
        FIXTURES_REQUIRED "union_output"
)