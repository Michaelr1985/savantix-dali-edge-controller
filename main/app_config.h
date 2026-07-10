#pragma once

#include <cstdint>

struct SmartLightConfig final {
    static constexpr std::uint8_t kMaximumDaliLights = 64;

    int daliTxGpio{-1};
    int daliRxGpio{-1};
    int busPowerGpio{-1};
    int transceiverFaultGpio{-1};
    int busVoltageAdcGpio{-1};
    int busCurrentAdcGpio{-1};
    int c6TxGpio{-1};
    int c6RxGpio{-1};
    int c6RtsGpio{-1};
    int c6CtsGpio{-1};
    std::uint8_t maximumLights{64};
    std::uint32_t onStatusPollMs{5000};
    std::uint32_t offStatusPollMs{30000};
    std::uint32_t warningStatusPollMs{2000};
    std::uint8_t maxCommandRetries{3};
    float maximumNormalBusUtilisation{0.30F};
    float maximumDiagnosticBusUtilisation{0.60F};
    float temperatureWarningC{80.0F};
    float temperatureCriticalC{90.0F};
    std::uint8_t warningPersistenceSamples{3};
    std::uint8_t faultPersistenceSamples{5};
    std::uint8_t recoveryPersistenceSamples{10};
    bool c6Enabled{false};
    bool demoEnabled{true};
    bool anomalyInterfaceEnabled{false};

    [[nodiscard]] static SmartLightConfig defaults() noexcept;
    [[nodiscard]] static SmartLightConfig fromKconfig() noexcept;
    [[nodiscard]] bool validate() const noexcept;
};
