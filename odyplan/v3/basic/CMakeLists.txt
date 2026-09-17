add_subdirectory(io)
add_subdirectory(expression)
add_subdirectory(window)
add_subdirectory(operators)
set(FACADE_TARGET odyplan_basic)

add_library(${FACADE_TARGET} STATIC)
add_library(odyplan::basic ALIAS ${FACADE_TARGET})

target_link_libraries(${FACADE_TARGET} PUBLIC
        odyplan::basic::io
        odyplan::basic::expression
        odyplan::basic::window
        odyplan::basic::operators
        odyplan::core
)

add_custom_target(odyplan_basic_all)
add_dependencies(odyplan_basic_all
        odyplan_basic_io
        odyplan_basic_expression
        odyplan_basic_window
        odyplan_basic_operators
)