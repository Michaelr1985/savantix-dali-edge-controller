#include "dali/storage/local_storage.h"

#include <cstddef>

namespace dali::storage {

std::uint16_t EventHistory::checksum(const StoredEvent& event) noexcept {
    constexpr std::size_t length = offsetof(StoredEvent, crc);
    const auto* bytes = reinterpret_cast<const std::uint8_t*>(&event);
    std::uint16_t value = 0xFFFFU;
    for (std::size_t i = 0; i < length; ++i) {
        value ^= bytes[i];
        for (int bit = 0; bit < 8; ++bit) {
            value = (value & 1U) != 0U
                ? static_cast<std::uint16_t>((value >> 1U) ^ 0xA001U)
                : static_cast<std::uint16_t>(value >> 1U);
        }
    }
    return value;
}

bool EventHistory::append(StoredEvent event) noexcept {
    event.crc = 0;
    event.crc = checksum(event);
    const std::size_t index = (head_ + size_) % kCapacity;
    records_[index] = event;
    if (size_ < kCapacity) ++size_;
    else head_ = (head_ + 1U) % kCapacity;
    return true;
}

RecoveredEvents EventHistory::recover() const noexcept {
    RecoveredEvents recovered{};
    for (std::size_t i = 0; i < size_; ++i) {
        const StoredEvent& event = records_[(head_ + i) % kCapacity];
        if (event.crc == checksum(event)) {
            recovered.records[recovered.size++] = event;
        }
    }
    return recovered;
}

void EventHistory::corruptNewestForTest() noexcept {
    if (size_ == 0U) return;
    const std::size_t index = (head_ + size_ - 1U) % kCapacity;
    records_[index].crc ^= 0xFFFFU;
}

}  // namespace dali::storage
