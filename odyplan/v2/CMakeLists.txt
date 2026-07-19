file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "src/*.cpp"
)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
        "include/*.h"
)

if(SOURCES)
    add_library(OdyPlanV2 STATIC ${SOURCES})
    set(ODYPLAN_VIS PUBLIC)
else()
    add_library(OdyPlanV2 INTERFACE)
    set(ODYPLAN_VIS INTERFACE)
endif()

target_include_directories(OdyPlanV2 ${ODYPLAN_VIS}
        "include"
)

target_include_directories(OdyPlanV2 SYSTEM ${ODYPLAN_VIS}
        ${LLVM_INCLUDE_DIRS}
        ${CLANG_INCLUDE_DIRS}
)

target_compile_definitions(OdyPlanV2 ${ODYPLAN_VIS}
        ${LLVM_DEFINITIONS}
)

target_compile_options(OdyPlanV2 ${ODYPLAN_VIS}
        -fno-rtti
)

target_link_libraries(OdyPlanV2 ${ODYPLAN_VIS}
        nlohmann_json::nlohmann_json
        spdlog::spdlog_header_only
        -Wl,--start-group
        ${LLVM_LIBS}
        -Wl,--end-group
)

enable_clang_tidy_on_target(OdyPlanV2)

if(ENABLE_TOOLING)
    register_lint_sources(${SOURCES} ${HEADERS})
    register_tidy_sources(${SOURCES})

    if(ENABLE_CLANG_TIDY)
        add_header_tidy_check(odyplan_header_check
                HEADERS ${HEADERS}
                LINK_LIBS OdyPlanV2
        )
    endif()
endif ()

unset(ODYPLAN_VIS)