#include "app_config.h"

#ifdef ESP_PLATFORM
#include "sdkconfig.h"
#endif

SmartLightConfig SmartLightConfig::defaults() noexcept {
    return {};
}

SmartLightConfig SmartLightConfig::fromKconfig() noexcept {
#ifdef ESP_PLATFORM
    SmartLightConfig config{};
    config.daliTxGpio = CONFIG_SAVANTIX_DALI_TX_GPIO;
    config.daliRxGpio = CONFIG_SAVANTIX_DALI_RX_GPIO;
    config.busPowerGpio = CONFIG_SAVANTIX_BUS_POWER_GPIO;
    config.transceiverFaultGpio = CONFIG_SAVANTIX_TRANSCEIVER_FAULT_GPIO;
    config.busVoltageAdcGpio = CONFIG_SAVANTIX_BUS_VOLTAGE_ADC_GPIO;
    config.busCurrentAdcGpio = CONFIG_SAVANTIX_BUS_CURRENT_ADC_GPIO;
    config.c6TxGpio = CONFIG_SAVANTIX_C6_TX_GPIO;
    config.c6RxGpio = CONFIG_SAVANTIX_C6_RX_GPIO;
    config.c6RtsGpio = CONFIG_SAVANTIX_C6_RTS_GPIO;
    config.c6CtsGpio = CONFIG_SAVANTIX_C6_CTS_GPIO;
    config.maximumLights = CONFIG_SAVANTIX_MAX_LIGHTS;
    config.onStatusPollMs = CONFIG_SAVANTIX_ON_STATUS_POLL_MS;
    config.offStatusPollMs = CONFIG_SAVANTIX_OFF_STATUS_POLL_MS;
    config.warningStatusPollMs = CONFIG_SAVANTIX_WARNING_STATUS_POLL_MS;
    config.maxCommandRetries = CONFIG_SAVANTIX_MAX_COMMAND_RETRIES;
    config.maximumNormalBusUtilisation =
        static_cast<float>(CONFIG_SAVANTIX_NORMAL_BUS_UTILISATION_PERCENT) / 100.0F;
    config.maximumDiagnosticBusUtilisation =
        static_cast<float>(CONFIG_SAVANTIX_DIAGNOSTIC_BUS_UTILISATION_PERCENT) / 100.0F;
    config.temperatureWarningC = CONFIG_SAVANTIX_TEMPERATURE_WARNING_C;
    config.temperatureCriticalC = CONFIG_SAVANTIX_TEMPERATURE_CRITICAL_C;
    config.warningPersistenceSamples = CONFIG_SAVANTIX_WARNING_PERSISTENCE_SAMPLES;
    config.faultPersistenceSamples = CONFIG_SAVANTIX_FAULT_PERSISTENCE_SAMPLES;
    config.recoveryPersistenceSamples = CONFIG_SAVANTIX_RECOVERY_PERSISTENCE_SAMPLES;
#ifdef CONFIG_SAVANTIX_C6_ENABLE
    config.c6Enabled = true;
#endif
#ifdef CONFIG_SAVANTIX_DEMO_ENABLE
    config.demoEnabled = true;
#else
    config.demoEnabled = false;
#endif
#ifdef CONFIG_SAVANTIX_ANOMALY_INTERFACE_ENABLE
    config.anomalyInterfaceEnabled = true;
#endif
    return config;
#else
    return defaults();
#endif
}

bool SmartLightConfig::validate() const noexcept {
    const bool lightCountValid = maximumLights > 0 &&
                                 maximumLights <= kMaximumDaliLights;
    const bool periodsValid = onStatusPollMs > 0 && offStatusPollMs > 0 &&
                              warningStatusPollMs > 0;
    const bool retryValid = maxCommandRetries <= 8;
    const bool utilisationValid = maximumNormalBusUtilisation > 0.0F &&
        maximumNormalBusUtilisation <= 1.0F &&
        maximumDiagnosticBusUtilisation >= maximumNormalBusUtilisation &&
        maximumDiagnosticBusUtilisation <= 1.0F;
    const bool temperatureValid = temperatureCriticalC > temperatureWarningC;
    const bool persistenceValid = warningPersistenceSamples > 0 &&
        faultPersistenceSamples > 0 && recoveryPersistenceSamples > 0;
    const bool daliPinsDistinct = daliTxGpio < 0 || daliRxGpio < 0 ||
                                  daliTxGpio != daliRxGpio;
    const bool c6PinsValid = !c6Enabled ||
        (c6TxGpio >= 0 && c6RxGpio >= 0 && c6TxGpio != c6RxGpio);
    return lightCountValid && periodsValid && retryValid && utilisationValid &&
           temperatureValid && persistenceValid && daliPinsDistinct &&
           c6PinsValid;
}
