#pragma once

#include "sim/i_simulator.h"

namespace physics::sim
{
class EmptySimulator final : public ISimulator
{
public:
    [[nodiscard]] const char* name() const noexcept override;
    void render_ui(const SimulatorFrameContext& context) override;
};
} // namespace physics::sim
