idf_component_register(SRC_DIRS "src"
                        INCLUDE_DIRS "include"
                        REQUIRES clock_macro
                        PRIV_REQUIRES driver clock_memory  wifi_service
                    ) 