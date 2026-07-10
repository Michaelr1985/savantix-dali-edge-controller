#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "dali/core/types.h"

namespace dali::storage {

struct StoredEvent final {
    std::uint32_t sequence{0};
    std::uint64_t timestamp{0};
    LightEvent event{};
    std::uint16_t crc{0};
};

struct RecoveredEvents final {
    std::array<StoredEvent, 128> records{};
    std::size_t size{0};
};

class EventHistory final {
public:
    static constexpr std::size_t kCapacity = 128;

    [[nodiscard]] bool append(StoredEvent event) noexcept;
    [[nodiscard]] RecoveredEvents recover() const noexcept;
    [[nodiscard]] constexpr std::size_t size() const noexcept { return size_; }
    void corruptNewestForTest() noexcept;

private:
    [[nodiscard]] static std::uint16_t checksum(const StoredEvent& event) noexcept;
    std::array<StoredEvent, kCapacity> records_{};
    std::size_t head_{0};
    std::size_t size_{0};
};

}  // namespace dali::storage
