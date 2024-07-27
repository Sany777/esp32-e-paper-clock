idf_component_register(SRC_DIRS "src"
                    INCLUDE_DIRS "include"
                    EMBED_FILES embedded_files/favicon.ico embedded_files/index.html  embedded_files/script.js  embedded_files/style.css
                    PRIV_REQUIRES additional_functions clock_memory
                    REQUIRES  clock_system esp_http_server clock_module
                    )
