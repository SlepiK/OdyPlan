add_subdirectory(shared)
add_subdirectory(logical)
add_subdirectory(physical)
add_subdirectory(builder)
add_subdirectory(runtime)

set(FACADE_TARGET odyplan_core)

add_library(${FACADE_TARGET} INTERFACE)
add_library(odyplan::core ALIAS ${FACADE_TARGET})

#if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include")
#    target_include_directories(${FACADE_TARGET} INTERFACE
#            "${CMAKE_CURRENT_SOURCE_DIR}/include"
#    )
#endif()

target_link_libraries(${FACADE_TARGET} INTERFACE
        odyplan::core::logical
        odyplan::core::physical
        odyplan::core::shared
        odyplan::core::builder
        odyplan::core::runtime
)

#option(ODYPLAN_V3_ENABLE_LOOPED_SOURCE "Enable looped source feature (v3)" OFF)
#if(ODYPLAN_V3_ENABLE_LOOPED_SOURCE)
#    target_compile_definitions(${FACADE_TARGET} INTERFACE
#            ENABLE_LOOPED_SOURCE
#    )
#endif()

add_custom_target(odyplan_core_all)
add_dependencies(odyplan_core_all
        odyplan_core_shared
        odyplan_core_logical
        odyplan_core_physical
        odyplan_core_builder
        odyplan_core_runtime
)

