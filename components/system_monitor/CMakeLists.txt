idf_component_register(
    SRCS "system_monitor.cpp" "system_monitor_task.cpp"
    INCLUDE_DIRS "include"
)
target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra -Werror)
