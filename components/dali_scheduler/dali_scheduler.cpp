#include "dali/scheduler/dali_scheduler.h"

namespace dali::scheduler {

void DaliScheduler::configure(std::uint8_t address, LightPollContext context,
                              std::uint64_t nowMs) noexcept {
    if (address >= kMaximumLights) return;
    Slot& slot = slots_[address];
    slot.configured = true;
    slot.context = context;
    slot.dueMs.fill(nowMs + 60000U);
    slot.dueMs[static_cast<std::size_t>(PollKind::Status)] = nowMs;
    slot.dueMs[static_cast<std::size_t>(PollKind::Presence)] = nowMs + 30000U;
    slot.dueMs[static_cast<std::size_t>(PollKind::Lifetime)] = nowMs + 21600000U;
    slot.dueMs[static_cast<std::size_t>(PollKind::Identity)] = nowMs + 86400000U;
}

PollPriority DaliScheduler::priorityFor(
    PollKind kind, const LightPollContext& context) noexcept {
    if (kind == PollKind::Status && (context.warningActive || context.recentFault)) {
        return PollPriority::Critical;
    }
    if (kind == PollKind::Status || kind == PollKind::Presence) {
        return PollPriority::High;
    }
    if (kind == PollKind::Identity || kind == PollKind::Lifetime) {
        return PollPriority::Low;
    }
    return PollPriority::Normal;
}

std::uint64_t DaliScheduler::intervalFor(
    PollKind kind, const LightPollContext& context,
    std::uint64_t nowMs) noexcept {
    switch (kind) {
        case PollKind::Status:
            if (context.warningActive || context.recentFault) {
                const std::uint64_t elapsed =
                    nowMs >= context.observationStartedMs
                        ? nowMs - context.observationStartedMs : 0;
                if (elapsed < 60000U) return 2000U;
                if (elapsed < 360000U) return 5000U;
            }
            return context.commandedOn ? 5000U : 30000U;
        case PollKind::Presence: return 30000U;
        case PollKind::Electrical:
            return (context.warningActive || context.recentFault) ? 10000U : 60000U;
        case PollKind::Temperature:
            return context.warningActive ? 10000U : 60000U;
        case PollKind::Lifetime: return 21600000U;
        case PollKind::Identity: return 86400000U;
        case PollKind::Count: return 0U;
    }
    return 0U;
}

bool DaliScheduler::admit(const PollRequest& request, float utilisation,
                          float normalLimit, float diagnosticLimit) noexcept {
    const float limit = request.priority == PollPriority::Critical
                            ? diagnosticLimit : normalLimit;
    return utilisation >= 0.0F && utilisation < limit &&
           request.estimatedDurationMs > 0U;
}

std::optional<PollRequest> DaliScheduler::next(
    std::uint64_t nowMs, float utilisation, float normalLimit,
    float diagnosticLimit) noexcept {
    for (std::size_t offset = 0; offset < kMaximumLights; ++offset) {
        const std::size_t address =
            (static_cast<std::size_t>(roundRobinCursor_) + offset) % kMaximumLights;
        Slot& slot = slots_[address];
        if (!slot.configured) continue;
        for (std::size_t kindIndex = 0;
             kindIndex < static_cast<std::size_t>(PollKind::Count); ++kindIndex) {
            if (slot.dueMs[kindIndex] > nowMs) continue;
            const PollKind kind = static_cast<PollKind>(kindIndex);
            const PollRequest request{
                static_cast<std::uint8_t>(address), kind,
                priorityFor(kind, slot.context), 100U};
            if (!admit(request, utilisation, normalLimit, diagnosticLimit)) continue;
            roundRobinCursor_ = static_cast<std::uint8_t>((address + 1U) % kMaximumLights);
            return request;
        }
    }
    return std::nullopt;
}

void DaliScheduler::complete(const PollRequest& request,
                             std::uint64_t nowMs) noexcept {
    if (request.address >= kMaximumLights ||
        request.kind == PollKind::Count) return;
    Slot& slot = slots_[request.address];
    if (!slot.configured) return;
    const auto index = static_cast<std::size_t>(request.kind);
    slot.dueMs[index] = nowMs + intervalFor(request.kind, slot.context, nowMs);
}

}  // namespace dali::scheduler
