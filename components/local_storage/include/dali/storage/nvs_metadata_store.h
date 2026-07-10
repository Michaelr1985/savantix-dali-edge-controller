#pragma once

#include <cstdint>

#include "esp_err.h"

namespace dali::storage {

class NvsMetadataStore final {
public:
    [[nodiscard]] esp_err_t init() noexcept;
    [[nodiscard]] esp_err_t saveUint32(const char* key,
                                       std::uint32_t value) noexcept;
    [[nodiscard]] esp_err_t loadUint32(const char* key,
                                       std::uint32_t& value) noexcept;
};

}  // namespace dali::storage
