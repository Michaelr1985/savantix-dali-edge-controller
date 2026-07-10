idf_component_register(
    SRCS
        "dali_response_parser.cpp"
        "dali_transaction.cpp"
        "pipeline_query_client.cpp"
    INCLUDE_DIRS "include"
    REQUIRES dali_core dali_phy
)
target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror)
