cmake_minimum_required(VERSION 3.20)
project(savantix_dali_host_tests LANGUAGES CXX)

enable_testing()

add_executable(test_domain_types test_domain_types.cpp)
target_include_directories(test_domain_types PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_core/include
)
target_compile_features(test_domain_types PRIVATE cxx_std_17)
target_compile_options(test_domain_types PRIVATE -Wall -Wextra -Wpedantic -Werror)
add_test(NAME domain_types COMMAND test_domain_types)

add_executable(test_phy_contract test_phy_contract.cpp)
target_include_directories(test_phy_contract PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_phy/include
)
target_compile_features(test_phy_contract PRIVATE cxx_std_17)
target_compile_options(test_phy_contract PRIVATE -Wall -Wextra -Wpedantic -Werror)
add_test(NAME phy_contract COMMAND test_phy_contract)

add_library(dali_simulator_host STATIC
    ../../components/dali_simulator/simulated_dali_phy.cpp
)
target_include_directories(dali_simulator_host PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_phy/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_simulator/include
)
target_compile_features(dali_simulator_host PUBLIC cxx_std_17)
target_compile_options(dali_simulator_host PRIVATE
    -Wall -Wextra -Wpedantic -Werror)

foreach(simulator_test IN ITEMS
        test_simulated_dali_phy
        test_simulated_dali_phy_capacity)
    add_executable(${simulator_test} ${simulator_test}.cpp)
    target_include_directories(${simulator_test} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_phy/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_simulator/include
    )
    target_compile_features(${simulator_test} PRIVATE cxx_std_17)
    target_compile_options(${simulator_test} PRIVATE
        -Wall -Wextra -Wpedantic -Werror)
    target_link_libraries(${simulator_test} PRIVATE dali_simulator_host)
    add_test(NAME ${simulator_test} COMMAND ${simulator_test})
endforeach()

add_executable(test_time_contract test_time_contract.cpp)
target_include_directories(test_time_contract PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/time_service/include
)
target_compile_features(test_time_contract PRIVATE cxx_std_17)
target_compile_options(test_time_contract PRIVATE
    -Wall -Wextra -Wpedantic -Werror)
add_test(NAME time_contract COMMAND test_time_contract)

add_executable(test_foundation_composition
    test_foundation_composition.cpp
    ../../main/app_config.cpp
    ../../main/foundation_runtime.cpp
)
target_include_directories(test_foundation_composition PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../main
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_phy/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_simulator/include
)
target_compile_features(test_foundation_composition PRIVATE cxx_std_17)
target_compile_options(test_foundation_composition PRIVATE
    -Wall -Wextra -Wpedantic -Werror)
target_link_libraries(test_foundation_composition PRIVATE dali_simulator_host)
add_test(NAME foundation_composition COMMAND test_foundation_composition)

add_executable(test_dali_frame test_dali_frame.cpp)
target_include_directories(test_dali_frame PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_protocol/include
)
target_compile_features(test_dali_frame PRIVATE cxx_std_17)
target_compile_options(test_dali_frame PRIVATE
    -Wall -Wextra -Wpedantic -Werror)
add_test(NAME dali_frame COMMAND test_dali_frame)

add_executable(test_dali_control
    test_dali_control.cpp
    ../../components/dali_control/dali_control.cpp
    ../../components/dali_protocol/dali_transaction.cpp
)
target_include_directories(test_dali_control PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_phy/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_protocol/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_control/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_simulator/include
)
target_compile_features(test_dali_control PRIVATE cxx_std_17)
target_compile_options(test_dali_control PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_link_libraries(test_dali_control PRIVATE dali_simulator_host)
add_test(NAME dali_control COMMAND test_dali_control)

add_executable(test_dali_response_parser
    test_dali_response_parser.cpp
    ../../components/dali_protocol/dali_response_parser.cpp
)
target_include_directories(test_dali_response_parser PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_phy/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_protocol/include
)
target_compile_features(test_dali_response_parser PRIVATE cxx_std_17)
target_compile_options(test_dali_response_parser PRIVATE
    -Wall -Wextra -Wpedantic -Werror)
add_test(NAME dali_response_parser COMMAND test_dali_response_parser)

add_executable(test_dali_command_queue test_dali_command_queue.cpp)
target_include_directories(test_dali_command_queue PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_protocol/include
)
target_compile_features(test_dali_command_queue PRIVATE cxx_std_17)
target_compile_options(test_dali_command_queue PRIVATE
    -Wall -Wextra -Wpedantic -Werror)
add_test(NAME dali_command_queue COMMAND test_dali_command_queue)

add_executable(test_dali_transaction
    test_dali_transaction.cpp
    ../../components/dali_protocol/dali_transaction.cpp
)
target_include_directories(test_dali_transaction PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_phy/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_protocol/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_simulator/include
)
target_compile_features(test_dali_transaction PRIVATE cxx_std_17)
target_compile_options(test_dali_transaction PRIVATE
    -Wall -Wextra -Wpedantic -Werror)
target_link_libraries(test_dali_transaction PRIVATE dali_simulator_host)
add_test(NAME dali_transaction COMMAND test_dali_transaction)

add_executable(test_communication_quality
    test_communication_quality.cpp
    ../../components/dali_protocol/dali_transaction.cpp
)
target_include_directories(test_communication_quality PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_phy/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_protocol/include
)
target_compile_features(test_communication_quality PRIVATE cxx_std_17)
target_compile_options(test_communication_quality PRIVATE
    -Wall -Wextra -Wpedantic -Werror)
add_test(NAME communication_quality COMMAND test_communication_quality)

add_executable(test_dali_protocol_pipeline
    test_dali_protocol_pipeline.cpp
    ../../components/dali_protocol/dali_response_parser.cpp
    ../../components/dali_protocol/dali_transaction.cpp
)
target_include_directories(test_dali_protocol_pipeline PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_phy/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_protocol/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_simulator/include
)
target_compile_features(test_dali_protocol_pipeline PRIVATE cxx_std_17)
target_compile_options(test_dali_protocol_pipeline PRIVATE
    -Wall -Wextra -Wpedantic -Werror)
target_link_libraries(test_dali_protocol_pipeline PRIVATE dali_simulator_host)
add_test(NAME dali_protocol_pipeline COMMAND test_dali_protocol_pipeline)

add_executable(test_pipeline_query_client
    test_pipeline_query_client.cpp
    ../../components/dali_protocol/dali_response_parser.cpp
    ../../components/dali_protocol/dali_transaction.cpp
    ../../components/dali_protocol/pipeline_query_client.cpp
)
target_include_directories(test_pipeline_query_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_phy/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_protocol/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_simulator/include
)
target_compile_features(test_pipeline_query_client PRIVATE cxx_std_17)
target_compile_options(test_pipeline_query_client PRIVATE
    -Wall -Wextra -Wpedantic -Werror)
target_link_libraries(test_pipeline_query_client PRIVATE dali_simulator_host)
add_test(NAME pipeline_query_client COMMAND test_pipeline_query_client)

add_executable(test_dali_device_registry
    test_dali_device_registry.cpp
    ../../components/dali_device_manager/dali_device_manager.cpp
)
target_include_directories(test_dali_device_registry PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_phy/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_protocol/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_device_manager/include
)
target_compile_features(test_dali_device_registry PRIVATE cxx_std_17)
target_compile_options(test_dali_device_registry PRIVATE
    -Wall -Wextra -Wpedantic -Werror)
add_test(NAME dali_device_registry COMMAND test_dali_device_registry)

add_executable(test_dali_discovery
    test_dali_discovery.cpp
    ../../components/dali_device_manager/dali_device_manager.cpp
)
target_include_directories(test_dali_discovery PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_phy/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_protocol/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_device_manager/include
)
target_compile_features(test_dali_discovery PRIVATE cxx_std_17)
target_compile_options(test_dali_discovery PRIVATE
    -Wall -Wextra -Wpedantic -Werror)
add_test(NAME dali_discovery COMMAND test_dali_discovery)

add_executable(test_dali_capabilities
    test_dali_capabilities.cpp
    ../../components/dali_device_manager/dali_device_manager.cpp
)
target_include_directories(test_dali_capabilities PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_phy/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_protocol/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_device_manager/include
)
target_compile_features(test_dali_capabilities PRIVATE cxx_std_17)
target_compile_options(test_dali_capabilities PRIVATE
    -Wall -Wextra -Wpedantic -Werror)
add_test(NAME dali_capabilities COMMAND test_dali_capabilities)

add_executable(test_device_manager_integration
    test_device_manager_integration.cpp
    ../../components/dali_device_manager/dali_device_manager.cpp
)
target_include_directories(test_device_manager_integration PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_phy/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_protocol/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_device_manager/include
)
target_compile_features(test_device_manager_integration PRIVATE cxx_std_17)
target_compile_options(test_device_manager_integration PRIVATE
    -Wall -Wextra -Wpedantic -Werror)
add_test(NAME device_manager_integration COMMAND test_device_manager_integration)

add_executable(test_dali_scheduler
    test_dali_scheduler.cpp
    ../../components/dali_scheduler/dali_scheduler.cpp
)
target_include_directories(test_dali_scheduler PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_scheduler/include
)
target_compile_features(test_dali_scheduler PRIVATE cxx_std_17)
target_compile_options(test_dali_scheduler PRIVATE
    -Wall -Wextra -Wpedantic -Werror)
add_test(NAME dali_scheduler COMMAND test_dali_scheduler)

add_executable(test_light_health
    test_light_health.cpp
    ../../components/light_health_engine/light_health_engine.cpp
)
target_include_directories(test_light_health PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/light_health_engine/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/trend_engine/include
)
target_compile_features(test_light_health PRIVATE cxx_std_17)
target_compile_options(test_light_health PRIVATE
    -Wall -Wextra -Wpedantic -Werror)
add_test(NAME light_health COMMAND test_light_health)

add_executable(test_trend_engine
    test_trend_engine.cpp
    ../../components/trend_engine/trend_engine.cpp
)
target_include_directories(test_trend_engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/trend_engine/include
)
target_compile_features(test_trend_engine PRIVATE cxx_std_17)
target_compile_options(test_trend_engine PRIVATE
    -Wall -Wextra -Wpedantic -Werror)
add_test(NAME trend_engine COMMAND test_trend_engine)

add_executable(test_event_manager
    test_event_manager.cpp
    ../../components/event_manager/event_manager.cpp
)
target_include_directories(test_event_manager PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/event_manager/include
)
target_compile_features(test_event_manager PRIVATE cxx_std_17)
target_compile_options(test_event_manager PRIVATE
    -Wall -Wextra -Wpedantic -Werror)
add_test(NAME event_manager COMMAND test_event_manager)

add_executable(test_local_storage
    test_local_storage.cpp
    ../../components/local_storage/local_storage.cpp
)
target_include_directories(test_local_storage PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/local_storage/include
)
target_compile_features(test_local_storage PRIVATE cxx_std_17)
target_compile_options(test_local_storage PRIVATE
    -Wall -Wextra -Wpedantic -Werror)
add_test(NAME local_storage COMMAND test_local_storage)

add_executable(test_c6_interface
    test_c6_interface.cpp
    ../../components/c6_interface/c6_interface.cpp
)
target_include_directories(test_c6_interface PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/c6_interface/include
)
target_compile_features(test_c6_interface PRIVATE cxx_std_17)
target_compile_options(test_c6_interface PRIVATE
    -Wall -Wextra -Wpedantic -Werror)
add_test(NAME c6_interface COMMAND test_c6_interface)

add_executable(test_system_monitor
    test_system_monitor.cpp
    ../../components/system_monitor/system_monitor.cpp
)
target_include_directories(test_system_monitor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/system_monitor/include
)
target_compile_features(test_system_monitor PRIVATE cxx_std_17)
target_compile_options(test_system_monitor PRIVATE
    -Wall -Wextra -Wpedantic -Werror)
add_test(NAME system_monitor COMMAND test_system_monitor)

add_executable(test_demo_controller
    test_demo_controller.cpp
    ../../components/demo_controller/demo_controller.cpp
    ../../components/dali_device_manager/dali_device_manager.cpp
    ../../components/event_manager/event_manager.cpp
    ../../components/c6_interface/c6_interface.cpp
    ../../components/light_health_engine/light_health_engine.cpp
)
target_include_directories(test_demo_controller PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_phy/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_protocol/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_device_manager/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/event_manager/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/c6_interface/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/light_health_engine/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/demo_controller/include
)
target_compile_features(test_demo_controller PRIVATE cxx_std_17)
target_compile_options(test_demo_controller PRIVATE
    -Wall -Wextra -Wpedantic -Werror)
add_test(NAME demo_controller COMMAND test_demo_controller)

add_executable(test_diagnostics
    test_diagnostics.cpp
    ../../components/dali_diagnostics/diagnostics.cpp
)
target_include_directories(test_diagnostics PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_diagnostics/include
)
target_compile_features(test_diagnostics PRIVATE cxx_std_17)
target_compile_options(test_diagnostics PRIVATE
    -Wall -Wextra -Wpedantic -Werror)
add_test(NAME diagnostics COMMAND test_diagnostics)

add_executable(test_baseline_engine
    test_baseline_engine.cpp
    ../../components/baseline_engine/baseline_engine.cpp
)
target_include_directories(test_baseline_engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/dali_core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../components/baseline_engine/include
)
target_compile_features(test_baseline_engine PRIVATE cxx_std_17)
target_compile_options(test_baseline_engine PRIVATE
    -Wall -Wextra -Wpedantic -Werror)
add_test(NAME baseline_engine COMMAND test_baseline_engine)
