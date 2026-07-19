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

function(register_tidy_sources)
    if(NOT ARGN)
        return()
    endif()
    get_property(_current GLOBAL PROPERTY TIDY_SOURCE_FILES_GLOBAL)
    list(APPEND _current ${ARGN})
    set_property(GLOBAL PROPERTY TIDY_SOURCE_FILES_GLOBAL "${_current}")
endfunction()

function(register_lint_sources)
    if(NOT ARGN)
        return()
    endif()
    get_property(_current GLOBAL PROPERTY LINT_SOURCE_FILES_GLOBAL)
    list(APPEND _current ${ARGN})
    set_property(GLOBAL PROPERTY LINT_SOURCE_FILES_GLOBAL "${_current}")
endfunction()

function(add_header_tidy_check target_name)
    if(NOT (ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE))
        return()
    endif()

    set(multiValueArgs HEADERS LINK_LIBS)
    cmake_parse_arguments(ARG "" "" "${multiValueArgs}" ${ARGN})

    set(_wrapper_dir "${CMAKE_BINARY_DIR}/tidy_wrappers/${target_name}")
    file(MAKE_DIRECTORY ${_wrapper_dir})

    set(_wrapper_sources)
    foreach(header ${ARG_HEADERS})
        string(MD5 _hash "${header}")
        set(_wrapper_file "${_wrapper_dir}/hdr_${_hash}.cpp")

        if(NOT EXISTS ${_wrapper_file})
            file(WRITE ${_wrapper_file} "#include \"${header}\"\n")
        endif()

        list(APPEND _wrapper_sources ${_wrapper_file})
    endforeach()

    if(_wrapper_sources)
        add_library(${target_name} OBJECT EXCLUDE_FROM_ALL ${_wrapper_sources})
        target_link_libraries(${target_name} PRIVATE ${ARG_LINK_LIBS})
        enable_clang_tidy_on_target(${target_name})

        get_property(_current GLOBAL PROPERTY TIDY_HEADER_TARGETS_GLOBAL)
        list(APPEND _current ${target_name})
        set_property(GLOBAL PROPERTY TIDY_HEADER_TARGETS_GLOBAL "${_current}")
    endif()
endfunction()

function(finalize_lint_targets)
    get_property(_format_files GLOBAL PROPERTY LINT_SOURCE_FILES_GLOBAL)
    get_property(_tidy_files GLOBAL PROPERTY TIDY_SOURCE_FILES_GLOBAL)
    get_property(_header_targets GLOBAL PROPERTY TIDY_HEADER_TARGETS_GLOBAL)

    if(_format_files)
        list(REMOVE_DUPLICATES _format_files)
    endif()
    if(_tidy_files)
        list(REMOVE_DUPLICATES _tidy_files)
    endif()

    if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE AND _tidy_files)
        add_custom_target(tidy
                COMMAND ${CLANG_TIDY_EXE} -p=${CMAKE_BINARY_DIR} ${_tidy_files}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                COMMENT "Running clang-tidy over project .cpp sources"
                VERBATIM
        )
        if(_header_targets)
            add_dependencies(tidy ${_header_targets})
        endif()
    endif()

    if(ENABLE_CLANG_FORMAT_CHECK AND CLANG_FORMAT_EXE AND _format_files)
        add_custom_target(format
                COMMAND ${CLANG_FORMAT_EXE} -i ${_format_files}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                COMMENT "Running clang-format in-place"
                VERBATIM
        )
        add_custom_target(format-check
                COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${_format_files}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                COMMENT "Checking clang-format compliance"
                VERBATIM
        )
    endif()
endfunction()