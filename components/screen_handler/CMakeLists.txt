idf_component_register(SRC_DIRS "src"
                        INCLUDE_DIRS "include"
                        PRIV_REQUIRES toolbox MPU6500 epaper_adapter periodic_task clock_module wifi_service setting_server device_common device_common forecast_http_client sound_generator AHT21 adc_reader driver
                    ) 