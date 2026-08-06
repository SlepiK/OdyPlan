if(NOT LLVM_INSTALL_DIR)
    message(FATAL_ERROR
        "LLVM_INSTALL DIR not set!\n"
            "Please provide it via -DLLVM_INSTALL_DIR=/path/to/llvm"
    )
endif ()

if(NOT EXISTS "${LLVM_INSTALL_DIR}")
    message(FATAL_ERROR
            "LLVM_INSTALL_DIR does not exist: ${LLVM_INSTALL_DIR}\n"
            "Please check the path."
    )
endif()

set(LLVM_DIR  ${LLVM_INSTALL_DIR}/lib/cmake/llvm)
set(Clang_DIR ${LLVM_INSTALL_DIR}/lib/cmake/clang)

find_package(LLVM REQUIRED CONFIG PATHS "${LLVM_DIR}" NO_DEFAULT_PATH)
find_package(Clang REQUIRED CONFIG PATHS "${Clang_DIR}" NO_DEFAULT_PATH)

message(STATUS "FOUND LLVM ${LLVM_VERSION}: ${LLVM_DIR}")
message(STATUS "FOUND Clang ${Clang_VERSION}: ${Clang_DIR}")

llvm_map_components_to_libnames(LLVM_LIBS
        core
        support
        irreader
        native
        nativecodegen

        orcjit
)

if(LLVM_ENABLE_ABI_BREAKING_CHECKS)
    add_definitions(-DLLVM_ENABLE_ABI_BREAKING_CHECKS=1)
else()
    add_definitions(-DLLVM_ENABLE_ABI_BREAKING_CHECKS=0)
endif()

function(odyplan_link_llvm TARGET_NAME VISIBILITY)
    target_include_directories(${TARGET_NAME} SYSTEM ${VISIBILITY}
            ${LLVM_INCLUDE_DIRS}
    )

    separate_arguments(_llvm_defs_list UNIX_COMMAND "${LLVM_DEFINITIONS}")
    if(_llvm_defs_list)
        target_compile_definitions(${TARGET_NAME} ${VISIBILITY}
                ${_llvm_defs_list}
        )
    endif()

    if(MSVC)
        target_compile_options(${TARGET_NAME} ${VISIBILITY} /GR-)
    else()
        target_compile_options(${TARGET_NAME} ${VISIBILITY} -fno-rtti)
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_link_libraries(${TARGET_NAME} ${VISIBILITY}
                -Wl,--start-group
                ${LLVM_LIBS}
                -Wl,--end-group
        )
    else()
        target_link_libraries(${TARGET_NAME} ${VISIBILITY}
                ${LLVM_LIBS}
        )
    endif()
endfunction()