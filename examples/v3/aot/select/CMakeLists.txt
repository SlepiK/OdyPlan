file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

# 1. Compiler

set(COMPILER_TARGET_NAME odyplan_example_aot_select_compiler)

add_executable(${COMPILER_TARGET_NAME}
        src/select_compiler.cpp
)

target_include_directories(${COMPILER_TARGET_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(${COMPILER_TARGET_NAME} PRIVATE
        odyplan::core
        odyplan::basic
)

# 2. Generate o.-File of Plan

set(GENERATED_PLAN_OBJECT ${CMAKE_CURRENT_BINARY_DIR}/select_plan.o)

add_custom_command(
        OUTPUT ${GENERATED_PLAN_OBJECT}
        COMMAND ${COMPILER_TARGET_NAME} ${GENERATED_PLAN_OBJECT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${COMPILER_TARGET_NAME}
        COMMENT "Compiling logical select-plan to native object file"
)

add_custom_target(odyplan_example_aot_select_plan DEPENDS ${GENERATED_PLAN_OBJECT})

#3. Final Step

set(TARGET_NAME odyplan_example_aot_select)

add_executable(${TARGET_NAME}
        src/select_entry.cpp
        ${GENERATED_PLAN_OBJECT}
)

target_include_directories(${TARGET_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_dependencies(${TARGET_NAME}
        odyplan_example_aot_select_plan
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

if(ENABLE_TOOLING)
    register_lint_sources(${SOURCES} ${HEADERS})
    register_tidy_sources(${SOURCES})
endif()