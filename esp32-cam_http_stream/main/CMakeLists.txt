idf_component_register(SRCS "camera_httpd.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_http_server esp32-camera nvs_flash esp_wifi esp_timer esp_netif esp_psram)
