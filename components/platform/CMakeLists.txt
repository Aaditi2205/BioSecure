idf_component_register(SRCS "esp_uart_transport.cpp" "device_time.cpp" INCLUDE_DIRS "include" REQUIRES core r307 driver esp_timer esp_netif lwip)
