idf_component_register(SRC_DIRS "src"
                        INCLUDE_DIRS "include"
                        REQUIRES device_macro
                        PRIV_REQUIRES driver device_memory wifi_service device_system MPU6500 AHT21 epaper_adapter  adc_reader i2c_module  periodic_task sound_generator clock_module
                    ) 