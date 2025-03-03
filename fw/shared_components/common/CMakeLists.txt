set(component_srcs "common.c")

idf_component_register(SRCS "${component_srcs}"
                       PRIV_REQUIRES freertos driver
                       REQUIRES nvs_handler nvs_flash
                       INCLUDE_DIRS ".")

idf_build_get_property(mode COMMON_RELEASE_PLATFORM)
add_compile_definitions(COMMON_RELEASE_PLATFORM=${mode})
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error=switch -Wno-error=format=)