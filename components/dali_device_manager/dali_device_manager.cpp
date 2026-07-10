#include "dali/device/dali_device_manager.h"

namespace dali::device {
namespace {

using protocol::CommandPriority;
using protocol::DaliCommand;
using protocol::DaliQueryOutcome;

bool isValidResponse(const protocol::DaliQueryResult& result) noexcept {
    return result.outcome == DaliQueryOutcome::Ok && result.response.hasValue();
}

bool isBusFailure(const protocol::DaliQueryResult& result) noexcept {
    return result.outcome == DaliQueryOutcome::BusFailure ||
           result.phyResult == PhyResult::BusPowerLost;
}

}  // namespace

DaliDeviceRecord* DaliDeviceRegistry::find(ShortAddress address) noexcept {
    auto& slot = records_[address.value()];
    return slot.has_value() ? &*slot : nullptr;
}

const DaliDeviceRecord* DaliDeviceRegistry::find(
    ShortAddress address) const noexcept {
    const auto& slot = records_[address.value()];
    return slot.has_value() ? &*slot : nullptr;
}

DaliDeviceRecord& DaliDeviceRegistry::upsert(
    ShortAddress address,
    std::uint64_t nowMs) noexcept {
    auto& slot = records_[address.value()];
    if (!slot.has_value()) {
        slot.emplace(address, nowMs);
        ++count_;
    } else {
        slot->lastSeenUptimeMs = nowMs;
    }
    return *slot;
}

DiscoveryBatch DaliDeviceManager::scanPresence(std::uint64_t nowMs) noexcept {
    DiscoveryBatch batch{};
    static std::array<bool, DaliDeviceRegistry::kCapacity> duplicateReported{};

    for (std::uint8_t raw = 0; raw < DaliDeviceRegistry::kCapacity; ++raw) {
        const ShortAddress address = *ShortAddress::fromRaw(raw);
        const protocol::DaliQueryResult result = client_.query(
            address, DaliCommand::QueryControlGearPresent, CommandPriority::High);
        DaliDeviceRecord* record = registry_.find(address);

        if (isValidResponse(result)) {
            duplicateReported[raw] = false;
            if (record == nullptr) {
                record = &registry_.upsert(address, nowMs);
                record->lifecycle = DeviceLifecycle::Present;
                record->present = true;
                record->missedPresenceChecks = 0;
                batch.add({DiscoveryChangeType::DeviceDiscovered, address, nowMs});
            } else {
                const bool recovered = record->lifecycle == DeviceLifecycle::Missing;
                record->lifecycle = DeviceLifecycle::Present;
                record->present = true;
                record->missedPresenceChecks = 0;
                record->lastSeenUptimeMs = nowMs;
                if (recovered) {
                    batch.add({DiscoveryChangeType::DeviceRecovered, address, nowMs});
                }
            }
            continue;
        }

        if (result.outcome == DaliQueryOutcome::Collision) {
            if (record != nullptr) record->lifecycle = DeviceLifecycle::DuplicateAddress;
            if (!duplicateReported[raw]) {
                duplicateReported[raw] = true;
                batch.add({DiscoveryChangeType::DuplicateAddressSuspected,
                           address, nowMs});
            }
            continue;
        }

        if (record == nullptr || isBusFailure(result) ||
            record->lifecycle == DeviceLifecycle::Missing) {
            continue;
        }

        if (record->missedPresenceChecks < 3U) ++record->missedPresenceChecks;
        if (record->missedPresenceChecks >= 3U) {
            record->lifecycle = DeviceLifecycle::Missing;
            record->present = false;
            batch.add({DiscoveryChangeType::DeviceMissing, address, nowMs});
        }
    }
    return batch;
}

DiscoveryBatch DaliDeviceManager::detectCoreCapabilities(
    ShortAddress address, std::uint64_t nowMs) noexcept {
    DiscoveryBatch changes{};
    DaliDeviceRecord* record = registry_.find(address);
    if (record == nullptr) return changes;

    const auto probe = [&](CapabilityFeature feature, DaliCommand command,
                           Reading<std::uint8_t>* destination) noexcept {
        CapabilityEntry& entry = record->capabilities.at(feature);
        if (entry.state == CapabilityState::Unsupported) return;
        const protocol::DaliQueryResult result = client_.query(
            address, command, CommandPriority::High);
        if (isValidResponse(result)) {
            entry.state = CapabilityState::Supported;
            entry.consecutiveNoResponses = 0;
            if (destination != nullptr) {
                *destination = Reading<std::uint8_t>::valid(
                    result.response.value, nowMs, 0);
            }
            return;
        }
        if (isBusFailure(result) || result.outcome == DaliQueryOutcome::Collision) {
            return;
        }
        if (entry.consecutiveNoResponses < 2U) ++entry.consecutiveNoResponses;
        if (entry.consecutiveNoResponses >= 2U) {
            entry.state = CapabilityState::Unsupported;
        }
    };

    probe(CapabilityFeature::BasicStatus, DaliCommand::QueryStatus, nullptr);
    probe(CapabilityFeature::DeviceType, DaliCommand::QueryDeviceType,
          &record->deviceType);
    probe(CapabilityFeature::Version, DaliCommand::QueryVersionNumber,
          &record->version);
    probe(CapabilityFeature::PhysicalMinimum, DaliCommand::QueryPhysicalMinimum,
          &record->physicalMinimum);
    return changes;
}

}  // namespace dali::device
