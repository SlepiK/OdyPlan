cmake_minimum_required(VERSION 4.1)
project(OdyPlan)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

option(ENABLE_LLVM "Enable LLVM/Clang integration" OFF)
option(ENABLE_TOOLING "Enable clang-tidy/format targets" OFF)
option(ENABLE_CLANG_TIDY "Run clang-tidy as part of the build" OFF)
option(ENABLE_CLANG_FORMAT_CHECK "Enable clang-format/format-check targets" OFF)

include(cmake/dependencies.cmake)

if(ENABLE_LLVM OR ENABLE_TOOLING)
    include(cmake/llvm.cmake)
endif ()

if(ENABLE_TOOLING)
    include(cmake/tooling.cmake)
endif ()

add_subdirectory(odyplan)
add_subdirectory(examples)
