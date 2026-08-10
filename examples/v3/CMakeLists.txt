add_subdirectory(aot)
add_subdirectory(jit)
add_subdirectory(logical_plan)

add_custom_target(odyplan_example_all)
add_dependencies(odyplan_example_all
        odyplan_example_logical_plan
        odyplan_example_aot_all
        odyplan_example_jit_all
)