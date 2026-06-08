#pragma once

#include "sim/simulation_tick.h"
#include "sim/simulator_context.h"

namespace physics::sim
{
class ISimulator
{
public:
    virtual ~ISimulator() = default;

    [[nodiscard]] virtual const char* name() const noexcept = 0;
    virtual void update(const SimulationTick& tick) = 0;
    virtual void render_ui(const SimulatorFrameContext& context) = 0;
};
} // namespace physics::sim
