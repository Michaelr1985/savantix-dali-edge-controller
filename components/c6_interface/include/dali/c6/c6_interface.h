#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace dali::c6 {

enum class MessageType : std::uint8_t {
    LightEvent = 1,
    LightStatusResponse = 2,
    LightSummary = 3,
    DaliBusEvent = 4,
    DeviceDiscoveryEvent = 5,
    CommandAck = 6,
    CommandNack = 7,
    Heartbeat = 8,
};

struct Frame final {
    static constexpr std::size_t kMaximumPayload = 240;
    std::uint8_t version{1};
    MessageType type{MessageType::Heartbeat};
    std::uint16_t sequence{0};
    std::array<std::uint8_t, kMaximumPayload> payload{};
    std::uint16_t payloadLength{0};
};

class FrameCodec final {
public:
    static constexpr std::uint8_t kStart = 0x7EU;
    static constexpr std::uint8_t kEnd = 0x7FU;
    static constexpr std::size_t kMaximumEncodedSize = 250;

    [[nodiscard]] static bool encode(
        const Frame& frame,
        std::array<std::uint8_t, kMaximumEncodedSize>& output,
        std::size_t& outputLength) noexcept;
    [[nodiscard]] static bool decode(const std::uint8_t* bytes,
                                     std::size_t length,
                                     Frame& frame) noexcept;

private:
    [[nodiscard]] static std::uint16_t crc16(const std::uint8_t* bytes,
                                              std::size_t length) noexcept;
};

class IC6Transport {
public:
    virtual ~IC6Transport() = default;
    [[nodiscard]] virtual bool send(const std::uint8_t* bytes,
                                    std::size_t length) noexcept = 0;
};

class MockC6Transport final : public IC6Transport {
public:
    bool send(const std::uint8_t* bytes, std::size_t length) noexcept override;
    [[nodiscard]] std::size_t sentCount() const noexcept { return sentCount_; }
    [[nodiscard]] std::size_t lastLength() const noexcept { return lastLength_; }

private:
    std::array<std::uint8_t, FrameCodec::kMaximumEncodedSize> lastFrame_{};
    std::size_t lastLength_{0};
    std::size_t sentCount_{0};
};

class C6Session final {
public:
    C6Session(IC6Transport& transport, std::uint8_t maximumRetries) noexcept
        : transport_{transport}, maximumRetries_{maximumRetries} {}

    [[nodiscard]] bool publish(Frame frame) noexcept;
    [[nodiscard]] bool acknowledge(std::uint16_t sequence) noexcept;
    [[nodiscard]] Frame makeStatusRequest(std::uint8_t address) noexcept;

private:
    IC6Transport& transport_;
    std::uint8_t maximumRetries_{3};
    std::uint16_t nextSequence_{1};
    std::uint16_t pendingSequence_{0};
    bool pending_{false};
};

}  // namespace dali::c6
