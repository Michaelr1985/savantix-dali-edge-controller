#pragma once

#include <cstdint>

namespace dali::protocol {

enum class DaliCommand : std::uint8_t {
    Off = 0x00,
    QueryStatus = 0x90,
    QueryControlGearPresent = 0x91,
    QueryLampFailure = 0x92,
    QueryLampPowerOn = 0x93,
    QueryLimitError = 0x94,
    QueryResetState = 0x95,
    QueryMissingShortAddress = 0x96,
    QueryVersionNumber = 0x97,
    QueryDeviceType = 0x99,
    QueryPhysicalMinimum = 0x9A,
    QueryActualLevel = 0xA0,
    QueryMaxLevel = 0xA1,
    QueryMinLevel = 0xA2,
    QueryPowerOnLevel = 0xA3,
    QuerySystemFailureLevel = 0xA4,
    QueryFadeTimeRate = 0xA5,
    QueryGroups0To7 = 0xC0,
    QueryGroups8To15 = 0xC1,
    QueryRandomAddressH = 0xC2,
    QueryRandomAddressM = 0xC3,
    QueryRandomAddressL = 0xC4,
};

}  // namespace dali::protocol
