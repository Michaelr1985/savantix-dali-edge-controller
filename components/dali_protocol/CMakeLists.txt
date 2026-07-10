idf_component_register(
    INCLUDE_DIRS "include"
    REQUIRES dali_core dali_phy
)
target_compile_features(${COMPONENT_LIB} INTERFACE cxx_std_17)
