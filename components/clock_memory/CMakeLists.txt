idf_component_register(SRC_DIRS "src"
                        INCLUDE_DIRS "include"
                        PRIV_REQUIRES esp_wifi nvs_flash additional_functions clock_system
                    ) 