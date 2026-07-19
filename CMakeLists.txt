cmake_minimum_required(VERSION 4.1)
project(OdyPlan)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

option(ENABLE_TOOLING "Enable clang-tidy/format targets" ON)
option(ENABLE_CLANG_TIDY "Run clang-tidy as part of the build" ON)
option(ENABLE_CLANG_FORMAT_CHECK "Enable clang-format/format-check targets" ON)

option(ODYPLAN_V1 "Run OdyPlanV1 as part of the build" OFF)
option(ODYPLAN_V2 "Run OdyPlanV2 as part of the build" ON)

define_property(GLOBAL PROPERTY LINT_SOURCE_FILES_GLOBAL
        BRIEF_DOCS "Aggregated list of source and header files for linting"
        FULL_DOCS "..."
)
set_property(GLOBAL PROPERTY LINT_SOURCE_FILES_GLOBAL "")

define_property(GLOBAL PROPERTY TIDY_SOURCE_FILES_GLOBAL
        BRIEF_DOCS "Aggregated .cpp files for clang-tidy (NO headers!)"
        FULL_DOCS "..."
)
set_property(GLOBAL PROPERTY TIDY_SOURCE_FILES_GLOBAL "")

define_property(GLOBAL PROPERTY TIDY_HEADER_TARGETS_GLOBAL
        BRIEF_DOCS "Names of header-check object targets for clang-tidy"
        FULL_DOCS "..."
)
set_property(GLOBAL PROPERTY TIDY_HEADER_TARGETS_GLOBAL "")

include(cmake/dependencies.cmake)

include(cmake/llvm.cmake)

if(ENABLE_TOOLING)
    include(cmake/tooling.cmake)
endif ()

add_subdirectory(odyplan)
add_subdirectory(examples)

if(ENABLE_TOOLING)
    finalize_lint_targets()
endif ()
