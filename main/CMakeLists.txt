set(LV_EDITOR_PROJECT_SOURCES)

# set(COMPONENT_ADD_DEFINITIONS "-DLV_LVGL_H_INCLUDE_SIMPLE=1")
include(${CMAKE_CURRENT_LIST_DIR}/ui/component_lib_list_gen.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/ui/file_list_gen.cmake)
# add_library(lib-ui ${LV_EDITOR_PROJECT_SOURCES})

idf_component_register(SRCS "sd.c" "main.c" "can_manager.c" "io_extension.c" "i2c.c" "gpio.c" ${LV_EDITOR_PROJECT_SOURCES}
                    INCLUDE_DIRS ".")

target_compile_definitions(${COMPONENT_TARGET} PRIVATE LV_LVGL_H_INCLUDE_SIMPLE)
#target_compile_options(${COMPONENT_TARGET} PRIVATE -ULV_USE_XML)
idf_build_set_property(COMPILE_OPTIONS "-Wno-error=attributes" APPEND)