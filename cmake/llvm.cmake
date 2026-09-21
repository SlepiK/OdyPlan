
if(LLVM_INSTALL_DIR)
    if(NOT EXISTS "${LLVM_INSTALL_DIR}")
        message(FATAL_ERROR "LLVM_INSTALL_DIR does not exist: ${LLVM_INSTALL_DIR}")
    endif()

    set(LLVM_DIR ${LLVM_INSTALL_DIR}/lib/cmake/llvm)
    set(Clang_DIR ${LLVM_INSTALL_DIR}/lib/cmake/clang)

    find_package(LLVM REQUIRED CONFIG PATHS "${LLVM_DIR}" NO_DEFAULT_PATH)
    find_package(Clang REQUIRED CONFIG PATHS "${Clang_DIR}" NO_DEFAULT_PATH)

    message(STATUS "Found LLVM ${LLVM_VERSION} (explicit): ${LLVM_DIR}")
else()
    find_package(LLVM REQUIRED CONFIG)
    find_package(Clang REQUIRED CONFIG)

    message(STATUS "Found LLVM ${LLVM_VERSION} (system): ${LLVM_DIR}")
endif()

message(STATUS "FOUND LLVM ${LLVM_VERSION}: ${LLVM_DIR}")
message(STATUS "FOUND Clang ${Clang_VERSION}: ${Clang_DIR}")

# JIT

llvm_map_components_to_libnames(LLVM_LIBS
        core
        support
        irreader
        native
        nativecodegen
        orcjit
)

#AoT

set(ODYPLAN_AOT_SUPPORTED_TARGETS
        X86
        AArch64
        RISCV
)

llvm_map_components_to_libnames(LLVM_AOT_LIBS
        core
        support
        irreader
        asmprinter
        asmparser
        orcjit
        ${ODYPLAN_AOT_SUPPORTED_TARGETS}
)

if(LLVM_ENABLE_ABI_BREAKING_CHECKS)
    add_definitions(-DLLVM_ENABLE_ABI_BREAKING_CHECKS=1)
else()
    add_definitions(-DLLVM_ENABLE_ABI_BREAKING_CHECKS=0)
endif()

function(odyplan_llvm_include_dirs TARGET_NAME VISIBILITY)
    target_include_directories(${TARGET_NAME} SYSTEM ${VISIBILITY}
            ${LLVM_INCLUDE_DIRS}
    )

    separate_arguments(_llvm_defs_list NATIVE_COMMAND "${LLVM_DEFINITIONS}")
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
endfunction()

function(odyplan_link_llvm_jit TARGET_NAME VISIBILITY)
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

function(odyplan_link_llvm_aot TARGET_NAME VISIBILITY)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_link_libraries(${TARGET_NAME} ${VISIBILITY}
                -Wl,--start-group
                ${LLVM_AOT_LIBS}
                -Wl,--end-group
        )
    else()
        target_link_libraries(${TARGET_NAME} ${VISIBILITY}
                ${LLVM_AOT_LIBS}
        )
    endif()

    foreach(target_name ${ODYPLAN_AOT_SUPPORTED_TARGETS})
        target_compile_definitions(${TARGET_NAME} ${VISIBILITY} ODYPLAN_TARGET_${target_name})
    endforeach()
endfunction()