file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

set(TARGET_NAME odyplan_core_builder)

add_library(${TARGET_NAME} STATIC
        ${SOURCES}
        ${HEADERS}
)
add_library(odyplan::core::builder ALIAS ${TARGET_NAME})

target_include_directories(${TARGET_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(${TARGET_NAME}
        PUBLIC
        odyplan::core::logical
        odyplan::core::physical
        odyplan::core::shared
        PRIVATE
        odyplan::core::internal
        nlohmann_json::nlohmann_json
        spdlog::spdlog
)

odyplan_link_llvm(${TARGET_NAME} PUBLIC)

if(WIN32)
    target_compile_definitions(${TARGET_NAME} PUBLIC
            NOMINMAX
            WIN32_LEAN_AND_MEAN
    )
endif()

if(MSVC)
    target_compile_options(${TARGET_NAME} PRIVATE
            /permissive-
            /Zc:__cplusplus
            /bigobj
    )
endif()

if(COMMAND enable_clang_tidy_on_target)
    enable_clang_tidy_on_target(${TARGET_NAME})
endif()

if(ENABLE_TOOLING)
    register_lint_sources(${SOURCES} ${HEADERS})
    register_tidy_sources(${SOURCES})

    if(ENABLE_CLANG_TIDY)
        add_header_tidy_check(odyplan_builder_header_check
                HEADERS ${HEADERS}
                LINK_LIBS ${TARGET_NAME}
        )
    endif()
endif()