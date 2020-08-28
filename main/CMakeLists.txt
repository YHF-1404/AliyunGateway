set(srcs
    "Src/nrf_comm_use.c"
    "Src/spi_master_use.c"
    "Src/app_main.c"
    "Src/LinkToAliyun.c"
    "Src/wifi_use.c"
    )


idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "Inc/")