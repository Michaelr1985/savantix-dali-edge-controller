idf_component_register(
    SRCS "demo_controller.cpp"
    INCLUDE_DIRS "include"
    REQUIRES dali_core dali_phy dali_protocol dali_device_manager event_manager c6_interface light_health_engine
)
target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror)
