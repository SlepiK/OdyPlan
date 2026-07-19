find_program(CLANG_TIDY_EXE NAMES clang-tidy)
find_program(CLANG_FORMAT_EXE NAMES clang-format)

if(ENABLE_CLANG_TIDY)
    if(NOT CLANG_TIDY_EXE)
        message(FATAL_ERROR "ENABLE_CLANG_TIDY=ON but clang-tidy not found!")
    endif()
    message(STATUS "clang-tidy enabled: ${CLANG_TIDY_EXE}")
endif()

if(ENABLE_CLANG_FORMAT_CHECK)
    if(NOT CLANG_FORMAT_EXE)
        message(FATAL_ERROR "ENABLE_CLANG_FORMAT_CHECK=ON but clang-format not found!")
    endif()
    message(STATUS "clang-format found: ${CLANG_FORMAT_EXE}")
endif()

function(enable_clang_tidy_on_target target)
    if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
        set_target_properties(${target} PROPERTIES
                CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-p=${CMAKE_BINARY_DIR}"
        )
    endif()
endfunction()

function(register_lint_sources)
    if(NOT ARGN)
        return()
    endif()
    get_property(_current GLOBAL PROPERTY LINT_SOURCE_FILES_GLOBAL)
    list(APPEND _current ${ARGN})
    set_property(GLOBAL PROPERTY LINT_SOURCE_FILES_GLOBAL "${_current}")
endfunction()

function(finalize_lint_targets)
    get_property(_all_files GLOBAL PROPERTY LINT_SOURCE_FILES_GLOBAL)
    list(REMOVE_DUPLICATES _all_files)

    if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE AND _all_files)
        add_custom_target(tidy
                COMMAND ${CLANG_TIDY_EXE} -p=${CMAKE_BINARY_DIR} ${_all_files}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                COMMENT "Running clang-tidy over all project sources/headers"
                VERBATIM
        )
    endif()

    if(ENABLE_CLANG_FORMAT_CHECK AND CLANG_FORMAT_EXE AND _all_files)
        add_custom_target(format
                COMMAND ${CLANG_FORMAT_EXE} -i ${_all_files}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                COMMENT "Running clang-format in-place"
                VERBATIM
        )
        add_custom_target(format-check
                COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${_all_files}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                COMMENT "Checking clang-format compliance"
                VERBATIM
        )
    endif()
endfunction()