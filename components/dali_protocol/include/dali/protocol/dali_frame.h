#pragma once

#include <cstdint>
#include <optional>

#include "dali/core/types.h"
#include "dali/protocol/dali_commands.h"

namespace dali::protocol {

struct ForwardFrame final {
    std::uint16_t raw{0};

    [[nodiscard]] constexpr std::uint8_t addressByte() const noexcept {
        return static_cast<std::uint8_t>((raw >> 8U) & 0xFFU);
    }

    [[nodiscard]] constexpr std::uint8_t dataByte() const noexcept {
        return static_cast<std::uint8_t>(raw & 0xFFU);
    }
};

class ArcLevel final {
public:
    [[nodiscard]] static constexpr std::optional<ArcLevel>
    fromRaw(std::uint16_t value) noexcept {
        return value <= 254U
            ? std::optional<ArcLevel>{ArcLevel{static_cast<std::uint8_t>(value)}}
            : std::nullopt;
    }

    [[nodiscard]] constexpr std::uint8_t value() const noexcept {
        return value_;
    }

private:
    explicit constexpr ArcLevel(std::uint8_t value) noexcept : value_{value} {}
    std::uint8_t value_;
};

[[nodiscard]] constexpr ForwardFrame makeFrame(std::uint8_t address,
                                               std::uint8_t data) noexcept {
    return {static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(address) << 8U) | data)};
}

[[nodiscard]] constexpr ForwardFrame makeShortCommand(
    ShortAddress address,
    DaliCommand command) noexcept {
    const auto addressByte = static_cast<std::uint8_t>(
        (static_cast<std::uint8_t>(address.value() << 1U)) | 0x01U);
    return makeFrame(addressByte, static_cast<std::uint8_t>(command));
}

[[nodiscard]] constexpr ForwardFrame makeShortArc(
    ShortAddress address,
    ArcLevel level) noexcept {
    const auto addressByte =
        static_cast<std::uint8_t>(address.value() << 1U);
    return makeFrame(addressByte, level.value());
}

[[nodiscard]] constexpr ForwardFrame makeGroupCommand(
    GroupAddress address,
    DaliCommand command) noexcept {
    const auto addressByte = static_cast<std::uint8_t>(
        0x81U | static_cast<std::uint8_t>(address.value() << 1U));
    return makeFrame(addressByte, static_cast<std::uint8_t>(command));
}

[[nodiscard]] constexpr ForwardFrame makeGroupArc(
    GroupAddress address,
    ArcLevel level) noexcept {
    const auto addressByte = static_cast<std::uint8_t>(
        0x80U | static_cast<std::uint8_t>(address.value() << 1U));
    return makeFrame(addressByte, level.value());
}

[[nodiscard]] constexpr ForwardFrame makeBroadcastCommand(
    DaliCommand command) noexcept {
    return makeFrame(0xFFU, static_cast<std::uint8_t>(command));
}

[[nodiscard]] constexpr ForwardFrame makeSpecialCommand(
    DaliSpecialCommand command, std::uint8_t parameter) noexcept {
    return makeFrame(static_cast<std::uint8_t>(command), parameter);
}

[[nodiscard]] constexpr ForwardFrame makeBroadcastArc(ArcLevel level) noexcept {
    return makeFrame(0xFEU, level.value());
}

}  // namespace dali::protocol
