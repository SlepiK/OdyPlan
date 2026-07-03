option(ENABLE_V1_EXAMPLES "Enable the v1 examples" OFF)
option(ENABLE_V2_EXAMPLES "Enable the v2 examples" ON)

if(ENABLE_V1_EXAMPLES)
    add_subdirectory(v1)
endif ()
if(ENABLE_V2_EXAMPLES)
    add_subdirectory(v2)
endif()