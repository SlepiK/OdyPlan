cmake_minimum_required(VERSION 4.1)
project(OdyPlan VERSION 0.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(ENABLE_EXAMPLES "Enable example targets" ON)
option(ENABLE_TOOLING "Enable clang-tidy/format targets" ON)
option(ENABLE_CLANG_TIDY "Run clang-tidy as part of the build" OFF)
option(ENABLE_CLANG_FORMAT_CHECK "Enable clang-format/format-check targets" OFF)

set(ODYPLAN_VERSION "v3" CACHE STRING "Which OdyPlan version to build")
set_property(CACHE ODYPLAN_VERSION PROPERTY STRINGS "v1" "v2" "v3")

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

if(ENABLE_EXAMPLES)
    add_subdirectory(examples)
endif ()

if(ENABLE_TOOLING)
    finalize_lint_targets()
endif ()

# Install and Export

install(EXPORT OdyPlanTargets
        FILE OdyPlanTargets.cmake
        NAMESPACE sierra::odyplan::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OdyPlan
)

configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/OdyPlanConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/OdyPlanConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OdyPlan
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/OdyPlanConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/OdyPlanConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/OdyPlanConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OdyPlan
)

install(DIRECTORY odyplan/${ODYPLAN_VERSION}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/sierra/odyplan/${ODYPLAN_VERSION_DIR}
        FILES_MATCHING PATTERN "*.hpp"
        PATTERN "*.h"
)

message(STATUS "=== OdyPlan Build Done ===")
message(STATUS "Version: ${ODYPLAN_VERSION}")
message(STATUS "Install Prefix: ${CMAKE_INSTALL_PREFIX}")