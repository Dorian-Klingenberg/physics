#pragma once

#include "sim/i_simulator.h"

namespace physics::sim
{
class EmptySimulator final : public ISimulator
{
public:
    [[nodiscard]] const char* name() const noexcept override;
    void update(const SimulationTick& tick) override;
    void render_ui(const SimulatorFrameContext& context) override;

private:
    float elapsed_seconds_ = 0.0f;
    unsigned long long frame_count_ = 0;
};
} // namespace physics::sim
