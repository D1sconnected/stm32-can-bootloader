set(component_srcs "can_id.c")
idf_component_register(SRCS "${component_srcs}"
                       REQUIRES
                       INCLUDE_DIRS ".")

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error=switch)