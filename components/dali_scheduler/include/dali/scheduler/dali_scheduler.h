#pragma once

#include <array>
#include <cstdint>
#include <optional>

namespace dali::scheduler {

enum class PollKind : std::uint8_t {
    Status,
    Presence,
    Electrical,
    Temperature,
    Lifetime,
    Identity,
    Count,
};

enum class PollPriority : std::uint8_t {
    Critical,
    High,
    Normal,
    Low,
};

struct LightPollContext final {
    bool commandedOn{false};
    bool warningActive{false};
    bool recentFault{false};
    std::uint64_t observationStartedMs{0};
};

struct PollRequest final {
    std::uint8_t address{0};
    PollKind kind{PollKind::Status};
    PollPriority priority{PollPriority::Normal};
    std::uint32_t estimatedDurationMs{100};
};

class DaliScheduler final {
public:
    static constexpr std::size_t kMaximumLights = 64;

    void configure(std::uint8_t address, LightPollContext context,
                   std::uint64_t nowMs) noexcept;

    [[nodiscard]] std::optional<PollRequest> next(
        std::uint64_t nowMs, float utilisation,
        float normalLimit, float diagnosticLimit) noexcept;

    void complete(const PollRequest& request, std::uint64_t nowMs) noexcept;

    [[nodiscard]] static bool admit(const PollRequest& request,
                                    float utilisation,
                                    float normalLimit,
                                    float diagnosticLimit) noexcept;

private:
    struct Slot final {
        bool configured{false};
        LightPollContext context{};
        std::array<std::uint64_t,
                   static_cast<std::size_t>(PollKind::Count)> dueMs{};
    };

    [[nodiscard]] static PollPriority priorityFor(
        PollKind kind, const LightPollContext& context) noexcept;
    [[nodiscard]] static std::uint64_t intervalFor(
        PollKind kind, const LightPollContext& context,
        std::uint64_t nowMs) noexcept;

    std::array<Slot, kMaximumLights> slots_{};
    std::uint8_t roundRobinCursor_{0};
};

}  // namespace dali::scheduler
