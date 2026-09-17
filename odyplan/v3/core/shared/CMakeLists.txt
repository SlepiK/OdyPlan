file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

set(TARGET_NAME odyplan_core_shared)

add_library(${TARGET_NAME} INTERFACE)
add_library(odyplan::core::shared ALIAS ${TARGET_NAME})

target_include_directories(${TARGET_NAME} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

odyplan_llvm_include_dirs(${TARGET_NAME} INTERFACE)

if(WIN32)
    target_compile_definitions(${TARGET_NAME} INTERFACE
            NOMINMAX
            WIN32_LEAN_AND_MEAN
    )
endif()

if(MSVC)
    target_compile_options(${TARGET_NAME} INTERFACE
            /permissive-
            /Zc:__cplusplus
            /bigobj
    )
endif()

if(COMMAND enable_clang_tidy_on_target)
    enable_clang_tidy_on_target(${TARGET_NAME})
endif()

if(ENABLE_TOOLING)
    register_lint_sources(${HEADERS})
    register_tidy_sources()

    if(ENABLE_CLANG_TIDY)
        add_header_tidy_check(odyplan_shared_header_check
                HEADERS ${HEADERS}
                LINK_LIBS ${TARGET_NAME}
        )
    endif()
endif()