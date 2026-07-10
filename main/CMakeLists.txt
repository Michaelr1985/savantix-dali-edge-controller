idf_component_register(
    SRCS
        "app_main.cpp"
        "app_config.cpp"
        "foundation_runtime.cpp"
    INCLUDE_DIRS "."
    REQUIRES
        dali_core
        dali_phy
        dali_protocol
        dali_simulator
        time_service
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror)
