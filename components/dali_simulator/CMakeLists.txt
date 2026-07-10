idf_component_register(
    SRCS "simulated_dali_phy.cpp"
    INCLUDE_DIRS "include"
    REQUIRES dali_phy
)
target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror)
