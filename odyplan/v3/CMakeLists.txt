project(OdyPlanV3
        VERSION 3.0.0
        LANGUAGES CXX
)

set(PROJECT_VERSION ${PROJECT_VERSION} PARENT_SCOPE)
set(PROJECT_VERSION_MAJOR ${PROJECT_VERSION_MAJOR} PARENT_SCOPE)
set(PROJECT_VERSION_MINOR ${PROJECT_VERSION_MINOR} PARENT_SCOPE)
set(PROJECT_VERSION_PATCH ${PROJECT_VERSION_PATCH} PARENT_SCOPE)

add_subdirectory(core)
add_subdirectory(basic)

install(TARGETS odyplan_core odyplan_basic
        EXPORT OdyPlanTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(TARGETS
        odyplan_core_shared
        odyplan_core_logical
        odyplan_core_physical
        odyplan_core_builder
        odyplan_core_runtime

        EXPORT OdyPlanTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(TARGETS
        odyplan_basic_operators
        odyplan_basic_expression
        odyplan_basic_window
        odyplan_basic_io

        EXPORT OdyPlanTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)