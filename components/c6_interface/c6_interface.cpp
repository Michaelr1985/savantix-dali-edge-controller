#include "dali/c6/c6_interface.h"

#include <cstring>

namespace dali::c6 {

std::uint16_t FrameCodec::crc16(const std::uint8_t* bytes,
                                std::size_t length) noexcept {
    std::uint16_t crc = 0xFFFFU;
    for (std::size_t index = 0; index < length; ++index) {
        crc ^= bytes[index];
        for (int bit = 0; bit < 8; ++bit) {
            crc = (crc & 1U) != 0U
                ? static_cast<std::uint16_t>((crc >> 1U) ^ 0xA001U)
                : static_cast<std::uint16_t>(crc >> 1U);
        }
    }
    return crc;
}

bool FrameCodec::encode(
    const Frame& frame, std::array<std::uint8_t, kMaximumEncodedSize>& output,
    std::size_t& outputLength) noexcept {
    if (frame.payloadLength > Frame::kMaximumPayload) return false;
    const std::size_t required = 10U + frame.payloadLength;
    if (required > output.size()) return false;
    output[0] = kStart;
    output[1] = frame.version;
    output[2] = static_cast<std::uint8_t>(frame.type);
    output[3] = static_cast<std::uint8_t>(frame.sequence & 0xFFU);
    output[4] = static_cast<std::uint8_t>(frame.sequence >> 8U);
    output[5] = static_cast<std::uint8_t>(frame.payloadLength & 0xFFU);
    output[6] = static_cast<std::uint8_t>(frame.payloadLength >> 8U);
    std::memcpy(output.data() + 7U, frame.payload.data(), frame.payloadLength);
    const std::uint16_t checksum = crc16(output.data() + 1U,
                                          6U + frame.payloadLength);
    const std::size_t crcOffset = 7U + frame.payloadLength;
    output[crcOffset] = static_cast<std::uint8_t>(checksum & 0xFFU);
    output[crcOffset + 1U] = static_cast<std::uint8_t>(checksum >> 8U);
    output[crcOffset + 2U] = kEnd;
    outputLength = crcOffset + 3U;
    return true;
}

bool FrameCodec::decode(const std::uint8_t* bytes, std::size_t length,
                        Frame& frame) noexcept {
    if (bytes == nullptr || length < 10U || bytes[0] != kStart ||
        bytes[length - 1U] != kEnd) return false;
    const std::uint16_t payloadLength =
        static_cast<std::uint16_t>(bytes[5] | (bytes[6] << 8U));
    if (payloadLength > Frame::kMaximumPayload || length != payloadLength + 10U) {
        return false;
    }
    const std::size_t crcOffset = 7U + payloadLength;
    const std::uint16_t expected =
        static_cast<std::uint16_t>(bytes[crcOffset] | (bytes[crcOffset + 1U] << 8U));
    if (expected != crc16(bytes + 1U, 6U + payloadLength)) return false;
    frame.version = bytes[1];
    frame.type = static_cast<MessageType>(bytes[2]);
    frame.sequence = static_cast<std::uint16_t>(bytes[3] | (bytes[4] << 8U));
    frame.payloadLength = payloadLength;
    std::memcpy(frame.payload.data(), bytes + 7U, payloadLength);
    return true;
}

bool MockC6Transport::send(const std::uint8_t* bytes, std::size_t length) noexcept {
    if (bytes == nullptr || length > lastFrame_.size()) return false;
    std::memcpy(lastFrame_.data(), bytes, length);
    lastLength_ = length;
    ++sentCount_;
    return true;
}

bool C6Session::publish(Frame frame) noexcept {
    if (maximumRetries_ == 0U) return false;
    if (frame.sequence == 0U) frame.sequence = nextSequence_++;
    std::array<std::uint8_t, FrameCodec::kMaximumEncodedSize> wire{};
    std::size_t wireLength = 0;
    if (!FrameCodec::encode(frame, wire, wireLength) ||
        !transport_.send(wire.data(), wireLength)) return false;
    pendingSequence_ = frame.sequence;
    pending_ = true;
    return true;
}

bool C6Session::acknowledge(std::uint16_t sequence) noexcept {
    if (!pending_ || sequence != pendingSequence_) return false;
    pending_ = false;
    return true;
}

Frame C6Session::makeStatusRequest(std::uint8_t address) noexcept {
    Frame frame{};
    frame.type = MessageType::LightStatusResponse;
    frame.sequence = nextSequence_++;
    frame.payload[0] = address;
    frame.payloadLength = 1U;
    return frame;
}

}  // namespace dali::c6
