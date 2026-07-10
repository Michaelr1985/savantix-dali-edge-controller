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
