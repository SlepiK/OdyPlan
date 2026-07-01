file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "src/*.cpp"
)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
        "include/*.h"
)

if(SOURCES)
    add_library(OdyPlanV1 STATIC ${SOURCES})
else()
    add_library(OdyPlanV1 INTERFACE)
endif()

if(SOURCES)
    target_include_directories(OdyPlanV1 PUBLIC
            "include"
    )

    target_link_libraries(OdyPlanV1 PUBLIC
            nlohmann_json::nlohmann_json
            spdlog::spdlog_header_only
    )
else()
    target_include_directories(OdyPlanV1 INTERFACE
            "include"
    )

    target_link_libraries(OdyPlanV1 INTERFACE
            nlohmann_json::nlohmann_json
            spdlog::spdlog_header_only
    )
endif()

set(LINT_SOURCE_FILES ${SOURCES} ${HEADERS})

if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE AND LINT_SOURCE_FILES)
    add_custom_target(tidy
            COMMAND ${CLANG_TIDY_EXE} -p=${CMAKE_BINARY_DIR} ${LINT_SOURCE_FILES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Running clang-tidy over project sources/headers"
            VERBATIM
    )
endif()

if(ENABLE_CLANG_FORMAT_CHECK AND CLANG_FORMAT_EXE AND LINT_SOURCE_FILES)
    add_custom_target(format
            COMMAND ${CLANG_FORMAT_EXE} -i ${LINT_SOURCE_FILES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Running clang-format in-place"
            VERBATIM
    )
    add_custom_target(format-check
            COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${LINT_SOURCE_FILES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Checking clang-format compliance"
            VERBATIM
    )
endif()