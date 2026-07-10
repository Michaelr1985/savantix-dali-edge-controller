idf_component_register(
    INCLUDE_DIRS "include"
    REQUIRES dali_core
)
target_compile_features(${COMPONENT_LIB} INTERFACE cxx_std_17)
