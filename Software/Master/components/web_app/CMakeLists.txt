idf_component_register(SRCS "web_app.c" "net_connection.c"
                    INCLUDE_DIRS "include"
                    REQUIRES spi_flash led_strip driver esp_http_server esp_wifi nvs_flash data_gather)
