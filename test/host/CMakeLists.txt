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
