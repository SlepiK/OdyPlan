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
