idf_component_register(SRC_DIRS "src"
                        INCLUDE_DIRS "include"
                        PRIV_REQUIRES nvs_flash additional_functions clock_system esp_event esp_wifi
                    ) 