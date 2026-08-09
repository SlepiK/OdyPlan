add_subdirectory(select)

add_custom_target(odyplan_example_aot_all)
add_dependencies(odyplan_example_aot_all
        odyplan_example_aot_select
)