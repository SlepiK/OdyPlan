include(FetchContent)

# spdlog
message(STATUS "Searching spdlog")
find_package(spdlog QUIET)
if(NOT spdlog_FOUND)
    FetchContent_Declare(
            spdlog
            GIT_REPOSITORY https://github.com/gabime/spdlog.git
            GIT_TAG v1.17.0
            SYSTEM
    )
    FetchContent_MakeAvailable(
            spdlog
    )
endif ()
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(SPDLOG_FMT_EXTERNAL OFF CACHE BOOL "" FORCE)

# nlohmann_json
message(STATUS "Searching nlohmann_json")
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    FetchContent_Declare(
            nlohmann_json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG v3.12.0
            SYSTEM
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# zstd
message(STATUS "Searching zstd")
find_package(zstd QUIET)
if(NOT zstd_FOUND)
    find_library(ZSTD_LIB NAMES zstd REQUIRED)
    add_library(zstd::libzstd_static UNKNOWN IMPORTED)
    set_target_properties(zstd::libzstd_static PROPERTIES
            IMPORTED_LOCATION "${ZSTD_LIB}"
    )
endif()
