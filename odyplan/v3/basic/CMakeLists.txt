add_subdirectory(io)
add_subdirectory(operators)
set(FACADE_TARGET odyplan_basic)

add_library(${FACADE_TARGET} INTERFACE)
add_library(odyplan::basic ALIAS ${FACADE_TARGET})

#if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include")
#    target_include_directories(${FACADE_TARGET} INTERFACE
#            "${CMAKE_CURRENT_SOURCE_DIR}/include"
#    )
#endif()

target_link_libraries(${FACADE_TARGET} INTERFACE
        odyplan::basic::io
        odyplan::basic::operators
)

#option(ODYPLAN_V3_ENABLE_LOOPED_SOURCE "Enable looped source feature (v3)" OFF)
#if(ODYPLAN_V3_ENABLE_LOOPED_SOURCE)
#    target_compile_definitions(${FACADE_TARGET} INTERFACE
#            ENABLE_LOOPED_SOURCE
#    )
#endif()

add_custom_target(odyplan_basic_all)
add_dependencies(odyplan_basic_all
        odyplan_basic_io
        odyplan_basic_operators
)

