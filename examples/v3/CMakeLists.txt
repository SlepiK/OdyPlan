add_subdirectory(aot)
add_subdirectory(jit)

add_custom_target(odyplan_example_all)
add_dependencies(odyplan_example_all
        odyplan_example_aot_all
        odyplan_example_jit_all
)