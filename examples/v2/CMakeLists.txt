option(ENABLE_LOOPED_SOURCE "Enable that source-operators running in a loop mode" OFF)

add_subdirectory(logical_plan)
add_subdirectory(full)
add_subdirectory(project)
add_subdirectory(select)
add_subdirectory(read_json)