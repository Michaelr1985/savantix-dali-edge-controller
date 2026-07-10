idf_component_register(
    SRCS "esp_time_source.cpp"
    INCLUDE_DIRS "include"
    REQUIRES esp_timer freertos
)
target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror)
