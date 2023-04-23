idf_component_register(SRCS "swaw_master_fw.c"
                    INCLUDE_DIRS "."
                    REQUIRES web_app)
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/led_strip)