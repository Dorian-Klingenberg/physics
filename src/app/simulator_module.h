#pragma once

#include <memory>
#include <stdexcept>

#include "gfx/i_simulator_renderer.h"
#include "sim/i_simulator.h"

namespace physics::app
{
struct SimulatorModule
{
    std::unique_ptr<sim::ISimulator> simulator;
    std::unique_ptr<gfx::ISimulatorRenderer> renderer;

    [[nodiscard]] sim::ISimulator& active_simulator() const
    {
        if (!simulator)
        {
            throw std::logic_error("SimulatorModule has no simulator.");
        }

        return *simulator;
    }

    [[nodiscard]] gfx::ISimulatorRenderer& active_renderer() const
    {
        if (!renderer)
        {
            throw std::logic_error("SimulatorModule has no renderer.");
        }

        return *renderer;
    }
};
} // namespace physics::app
