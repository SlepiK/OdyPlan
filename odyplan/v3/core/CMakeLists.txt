add_subdirectory(shared)
add_subdirectory(logical)
add_subdirectory(physical)
add_subdirectory(builder)
add_subdirectory(runtime)

set(FACADE_TARGET odyplan_core)

add_library(${FACADE_TARGET} STATIC)
add_library(odyplan::core ALIAS ${FACADE_TARGET})

target_link_libraries(${FACADE_TARGET} PUBLIC
        odyplan::core::logical
        odyplan::core::physical
        odyplan::core::shared
        odyplan::core::builder
        odyplan::core::runtime
)

add_custom_target(odyplan_core_all)
add_dependencies(odyplan_core_all
        odyplan_core_shared
        odyplan_core_logical
        odyplan_core_physical
        odyplan_core_builder
        odyplan_core_runtime
)