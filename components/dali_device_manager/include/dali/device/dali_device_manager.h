#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

#include "dali/core/types.h"
#include "dali/protocol/i_dali_query_client.h"

namespace dali::device {

enum class DeviceLifecycle : std::uint8_t {
    Unknown,
    Present,
    Missing,
    DuplicateAddress,
};

enum class CapabilityState : std::uint8_t {
    Unknown,
    Supported,
    Unsupported,
};

enum class CapabilityFeature : std::uint8_t {
    BasicStatus,
    DeviceType,
    Version,
    PhysicalMinimum,
    Energy,
    Power,
    Current,
    Temperature,
    OperatingHours,
    FailureCounters,
    Diagnostics,
    Identification,
    Count,
};

struct CapabilityEntry final {
    CapabilityState state{CapabilityState::Unknown};
    std::uint8_t consecutiveNoResponses{0};
};

class DaliCapabilityMap final {
public:
    [[nodiscard]] CapabilityEntry& at(CapabilityFeature feature) noexcept {
        return entries_[static_cast<std::size_t>(feature)];
    }

    [[nodiscard]] const CapabilityEntry& at(
        CapabilityFeature feature) const noexcept {
        return entries_[static_cast<std::size_t>(feature)];
    }

private:
    static constexpr std::size_t kFeatureCount =
        static_cast<std::size_t>(CapabilityFeature::Count);
    std::array<CapabilityEntry, kFeatureCount> entries_{};
};

struct DaliDeviceRecord final {
    explicit DaliDeviceRecord(ShortAddress shortAddress,
                              std::uint64_t nowMs) noexcept
        : address{shortAddress},
          firstSeenUptimeMs{nowMs},
          lastSeenUptimeMs{nowMs} {
        realtime.shortAddress = shortAddress.value();
    }

    ShortAddress address;
    DeviceLifecycle lifecycle{DeviceLifecycle::Unknown};
    bool present{false};
    std::uint8_t missedPresenceChecks{0};
    std::uint64_t firstSeenUptimeMs{0};
    std::uint64_t lastSeenUptimeMs{0};
    DaliCapabilityMap capabilities{};
    LightRealtimeData realtime{};
    LightLifetimeData lifetime{};
    LightHealthData health{};
    Reading<std::uint8_t> deviceType{};
    Reading<std::uint8_t> version{};
    Reading<std::uint8_t> physicalMinimum{};
};

enum class DiscoveryChangeType : std::uint8_t {
    DeviceDiscovered,
    DeviceMissing,
    DeviceRecovered,
    DuplicateAddressSuspected,
};

struct DiscoveryChange final {
    DiscoveryChangeType type{DiscoveryChangeType::DeviceDiscovered};
    ShortAddress address{*ShortAddress::fromRaw(0U)};
    std::uint64_t uptimeMs{0};
};

struct DiscoveryBatch final {
    static constexpr std::size_t kCapacity = 64;
    std::array<DiscoveryChange, kCapacity> changes{};
    std::size_t size{0};

    bool add(DiscoveryChange change) noexcept {
        if (size == kCapacity) return false;
        changes[size++] = change;
        return true;
    }
};

class DaliDeviceRegistry final {
public:
    static constexpr std::size_t kCapacity = 64;

    [[nodiscard]] DaliDeviceRecord* find(ShortAddress address) noexcept;
    [[nodiscard]] const DaliDeviceRecord* find(
        ShortAddress address) const noexcept;
    [[nodiscard]] DaliDeviceRecord& upsert(
        ShortAddress address,
        std::uint64_t nowMs) noexcept;
    [[nodiscard]] constexpr std::size_t count() const noexcept {
        return count_;
    }

private:
    std::array<std::optional<DaliDeviceRecord>, kCapacity> records_{};
    std::size_t count_{0};
};

class DaliDeviceManager final {
public:
    explicit DaliDeviceManager(protocol::IDaliQueryClient& client) noexcept
        : client_{client} {}

    [[nodiscard]] DiscoveryBatch scanPresence(std::uint64_t nowMs) noexcept;
    [[nodiscard]] DiscoveryBatch detectCoreCapabilities(
        ShortAddress address, std::uint64_t nowMs) noexcept;
    [[nodiscard]] DaliDeviceRegistry& registry() noexcept { return registry_; }
    [[nodiscard]] const DaliDeviceRegistry& registry() const noexcept {
        return registry_;
    }

private:
    protocol::IDaliQueryClient& client_;
    DaliDeviceRegistry registry_{};
};

}  // namespace dali::device
