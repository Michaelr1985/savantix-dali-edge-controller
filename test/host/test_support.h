#pragma once

#include <cstdlib>
#include <iostream>

#define CHECK_TRUE(expression) do { \
    if (!(expression)) { \
        std::cerr << __FILE__ << ':' << __LINE__ \
                  << " CHECK_TRUE failed: " #expression << '\n'; \
        return EXIT_FAILURE; \
    } \
} while (false)

#define CHECK_EQ(actual, expected) CHECK_TRUE((actual) == (expected))
