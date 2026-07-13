idf_component_register(
    SRCS "dali_control.cpp"
    INCLUDE_DIRS "include"
    REQUIRES dali_core dali_protocol
)
target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror)
