#include <array>
#include <cstdlib>

#include "dali/c6/c6_interface.h"
#include "test_support.h"

int main() {
    using namespace dali::c6;
    Frame frame{};
    frame.type = MessageType::LightEvent;
    frame.sequence = 42U;
    frame.payload[0] = 0xABU;
    frame.payloadLength = 1U;

    std::array<std::uint8_t, FrameCodec::kMaximumEncodedSize> wire{};
    std::size_t wireLength = 0;
    CHECK_TRUE(FrameCodec::encode(frame, wire, wireLength));
    CHECK_TRUE(wireLength > 8U);

    Frame decoded{};
    CHECK_TRUE(FrameCodec::decode(wire.data(), wireLength, decoded));
    CHECK_EQ(decoded.type, MessageType::LightEvent);
    CHECK_EQ(decoded.sequence, 42U);
    CHECK_EQ(decoded.payload[0], 0xABU);

    wire[wireLength - 2U] ^= 0x01U;
    CHECK_TRUE(!FrameCodec::decode(wire.data(), wireLength, decoded));

    MockC6Transport transport;
    C6Session session{transport, 3U};
    CHECK_TRUE(session.publish(frame));
    CHECK_EQ(transport.sentCount(), 1U);
    CHECK_TRUE(session.acknowledge(42U));
    CHECK_TRUE(!session.acknowledge(999U));

    const Frame status = session.makeStatusRequest(7U);
    CHECK_EQ(status.type, MessageType::LightStatusResponse);
    CHECK_EQ(status.payload[0], 7U);
    return EXIT_SUCCESS;
}
