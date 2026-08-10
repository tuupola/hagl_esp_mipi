idf_build_get_property(idf_version IDF_VERSION_MAJOR)

if(idf_version GREATER_EQUAL 6)
    set(driver_dependencies esp_driver_spi esp_driver_ledc esp_driver_gpio)
else()
    set(driver_dependencies driver)
endif()

idf_component_register(
    SRCS "src/hagl_hal_single.c" "src/hagl_hal_double.c" "src/hagl_hal_triple.c" "src/mipi_display.c"
    INCLUDE_DIRS "./include"
    REQUIRES hagl ${driver_dependencies}
)
