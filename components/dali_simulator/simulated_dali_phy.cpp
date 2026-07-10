#include "dali/sim/simulated_dali_phy.h"

namespace dali {

PhyResult SimulatedDaliPhy::init() noexcept {
    health_.initialised = true;
    return health_.transceiverHealthy ? PhyResult::Ok
                                      : PhyResult::TransceiverFault;
}

PhyResult SimulatedDaliPhy::transmit(std::uint16_t frame) noexcept {
    if (!health_.initialised) {
        return PhyResult::NotInitialised;
    }
    if (!health_.busPowered) {
        return PhyResult::BusPowerLost;
    }
    if (!health_.transceiverHealthy) {
        return PhyResult::TransceiverFault;
    }
    if (busBusy_) {
        return PhyResult::Busy;
    }
    if (collision_) {
        return PhyResult::Collision;
    }
    lastForwardFrame_ = frame;
    ++transmittedFrameCount_;
    return PhyResult::Ok;
}

PhyResult SimulatedDaliPhy::receive(std::uint8_t& frame,
                                    std::uint32_t timeoutMs) noexcept {
    if (!health_.initialised) {
        return PhyResult::NotInitialised;
    }
    if (!health_.busPowered) {
        return PhyResult::BusPowerLost;
    }
    if (!health_.transceiverHealthy) {
        return PhyResult::TransceiverFault;
    }
    if (responseCount_ == 0) {
        return PhyResult::Timeout;
    }

    const SimulatedResponse& response = responses_[responseHead_];
    if (response.delayMs > timeoutMs) {
        return PhyResult::Timeout;
    }

    const SimulatedResponse consumed = response;
    responseHead_ = (responseHead_ + 1U) % kResponseCapacity;
    --responseCount_;
    if (consumed.result == PhyResult::Ok) {
        frame = consumed.value;
    }
    return consumed.result;
}

bool SimulatedDaliPhy::isBusBusy() const noexcept {
    return busBusy_;
}

bool SimulatedDaliPhy::hasCollision() const noexcept {
    return collision_;
}

PhyHealth SimulatedDaliPhy::health() const noexcept {
    return health_;
}

PhyResult SimulatedDaliPhy::reset() noexcept {
    busBusy_ = false;
    collision_ = false;
    health_.lineStuckHigh = false;
    health_.lineStuckLow = false;
    return health_.transceiverHealthy ? PhyResult::Ok
                                      : PhyResult::TransceiverFault;
}

PhyResult SimulatedDaliPhy::enqueueResponse(
    SimulatedResponse response) noexcept {
    if (responseCount_ == kResponseCapacity) {
        return PhyResult::Busy;
    }
    const std::size_t tail =
        (responseHead_ + responseCount_) % kResponseCapacity;
    responses_[tail] = response;
    ++responseCount_;
    return PhyResult::Ok;
}

void SimulatedDaliPhy::setBusPowered(bool value) noexcept {
    health_.busPowered = value;
}

void SimulatedDaliPhy::setTransceiverHealthy(bool value) noexcept {
    health_.transceiverHealthy = value;
}

void SimulatedDaliPhy::setBusBusy(bool value) noexcept {
    busBusy_ = value;
}

void SimulatedDaliPhy::setCollision(bool value) noexcept {
    collision_ = value;
}

void SimulatedDaliPhy::setLineStuckHigh(bool value) noexcept {
    health_.lineStuckHigh = value;
}

void SimulatedDaliPhy::setLineStuckLow(bool value) noexcept {
    health_.lineStuckLow = value;
}

std::uint16_t SimulatedDaliPhy::lastForwardFrame() const noexcept {
    return lastForwardFrame_;
}

std::uint32_t SimulatedDaliPhy::transmittedFrameCount() const noexcept {
    return transmittedFrameCount_;
}

}  // namespace dali
