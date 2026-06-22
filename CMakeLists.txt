cmake_minimum_required(VERSION 4.1)
project(OdyPlan)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# --- Static analysis & formatting tooling -------------------------------
option(ENABLE_CLANG_TIDY "Run clang-tidy as part of the normal build" OFF)

find_program(CLANG_TIDY_EXE NAMES clang-tidy)
find_program(CLANG_FORMAT_EXE NAMES clang-format)

if(CLANG_TIDY_EXE)
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
else()
    message(STATUS "clang-tidy not found, tidy target(s) will be skipped")
endif()

if(CLANG_FORMAT_EXE)
    message(STATUS "clang-format found: ${CLANG_FORMAT_EXE}")
else()
    message(STATUS "clang-format not found, format target(s) will be skipped")
endif()

# If ENABLE_CLANG_TIDY is ON, every compile of every target defined below
# this point will also be run through clang-tidy. Off by default to keep
# normal incremental builds fast; flip on with -DENABLE_CLANG_TIDY=ON.
if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
    set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXE} -p=${CMAKE_BINARY_DIR})
endif()

message(STATUS "Searching spdlog")
find_package(spdlog QUIET)
if(NOT spdlog_FOUND)
    message(STATUS "spdlog not found, fetching...")
    FetchContent_Declare(
            spdlog
            GIT_REPOSITORY https://github.com/gabime/spdlog.git
            GIT_TAG v1.17.0
    )
    FetchContent_MakeAvailable(spdlog)
    message(STATUS "spdlog fetched")
else ()
    message(STATUS "spdlog found")
endif()

set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(SPDLOG_FMT_EXTERNAL OFF CACHE BOOL "" FORCE)

message(STATUS "Searching nlohmann_json")
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json not found, fetching...")
    FetchContent_Declare(
            nlohmann_json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG        v3.12.0
    )
    FetchContent_MakeAvailable(nlohmann_json)
    message(STATUS "nlohmann_json fetched")
else ()
    message(STATUS "nlohmann_json found")
endif()

message(STATUS "Searching zstd")
find_package(zstd QUIET)
if(NOT zstd_FOUND)
    message(STATUS "zstd not found, fetching...")
    find_library(ZSTD_LIB NAMES zstd REQUIRED)
    add_library(zstd::libzstd_static UNKNOWN IMPORTED)
    set_target_properties(zstd::libzstd_static PROPERTIES
            IMPORTED_LOCATION "${ZSTD_LIB}"
    )
else ()
    message(STATUS "zstd found")
endif()

if(NOT LLVM_INSTALL_DIR)
    set(LLVM_INSTALL_DIR /mnt/storage/ssdbig/llvm/llvm-22)
endif()

set(LLVM_DIR  ${LLVM_INSTALL_DIR}/lib/cmake/llvm)
set(Clang_DIR ${LLVM_INSTALL_DIR}/lib/cmake/clang)

find_package(LLVM REQUIRED CONFIG
        PATHS "${LLVM_DIR}"
        NO_DEFAULT_PATH)

find_package(Clang REQUIRED CONFIG
        PATHS "${Clang_DIR}"
        NO_DEFAULT_PATH)

message(STATUS "FOUND LLVM ${LLVM_VERSION}: ${LLVM_DIR}")
message(STATUS "FOUND Clang ${Clang_VERSION}: ${Clang_DIR}")

include_directories(${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

if(LLVM_ENABLE_ABI_BREAKING_CHECKS)
    add_definitions(-DLLVM_ENABLE_ABI_BREAKING_CHECKS=1)
else()
    add_definitions(-DLLVM_ENABLE_ABI_BREAKING_CHECKS=0)
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

# --- Main target ---------------------------------------------------------
# Falls back to a header-check OBJECT library when no .cpp files exist yet.
# OBJECT (not executable) is the key fix: each stub is compiled
# independently to verify the header is self-contained and well-formed,
# but nothing gets linked together — so no "multiple definition of main"
# when there's more than one header to check.
if(SOURCES)
    add_executable(OdyPlan ${SOURCES})
else()
    message(STATUS "No .cpp files found yet — generating header-check stub target (OBJECT, no link step)")

    set(HEADER_CHECK_DIR "${CMAKE_BINARY_DIR}/header_checks")
    file(MAKE_DIRECTORY ${HEADER_CHECK_DIR})

    set(HEADER_CHECK_SOURCES "")
    foreach(header ${HEADERS})
        get_filename_component(header_name ${header} NAME_WE)
        set(stub "${HEADER_CHECK_DIR}/${header_name}_check.cpp")
        # No main() here on purpose — OBJECT libraries are never linked,
        # so there's nothing to satisfy and nothing to collide.
        file(WRITE ${stub} "#include \"${header}\"\n")
        list(APPEND HEADER_CHECK_SOURCES ${stub})
    endforeach()

    if(NOT HEADER_CHECK_SOURCES)
        # No headers either — placeholder so the target always has
        # something to compile; this one DOES get an executable+main
        # since it's the only file and there's nothing to collide with.
        set(placeholder "${HEADER_CHECK_DIR}/placeholder.cpp")
        file(WRITE ${placeholder} "int main() { return 0; }\n")
        add_executable(OdyPlan ${placeholder})
    else()
        add_library(OdyPlan OBJECT ${HEADER_CHECK_SOURCES})
    endif()
endif()

target_compile_definitions(OdyPlan PRIVATE
        LLVM_CLANG_EXECUTABLE="${LLVM_INSTALL_DIR}/bin/clang"
        LLVM_RESOURCE_DIR="${LLVM_INSTALL_DIR}/lib/clang/${LLVM_VERSION_MAJOR}"
)

target_include_directories(OdyPlan PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(OdyPlan PRIVATE
        nlohmann_json::nlohmann_json
        spdlog::spdlog_header_only
)

# --- Tidy / format custom targets ----------------------------------------
# Independent of ENABLE_CLANG_TIDY above — these always exist (if the
# tools are installed) so you can run them on demand regardless of
# whether tidy-on-every-build is enabled.
set(LINT_SOURCE_FILES ${SOURCES} ${HEADERS})

if(CLANG_TIDY_EXE AND LINT_SOURCE_FILES)
    add_custom_target(tidy
            COMMAND ${CLANG_TIDY_EXE}
            -p=${CMAKE_BINARY_DIR}
            ${LINT_SOURCE_FILES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Running clang-tidy over project sources/headers"
            VERBATIM
    )
endif()

if(CLANG_FORMAT_EXE AND LINT_SOURCE_FILES)
    add_custom_target(format
            COMMAND ${CLANG_FORMAT_EXE} -i ${LINT_SOURCE_FILES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Running clang-format (in-place) over project sources/headers"
            VERBATIM
    )

    add_custom_target(format-check
            COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${LINT_SOURCE_FILES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Checking clang-format compliance (no files modified)"
            VERBATIM
    )
endif()