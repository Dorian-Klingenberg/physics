#include "app/default_simulator_module.h"

#include <memory>

#include "gfx/local_frame_grid_renderer.h"
#include "sim/local_frame_simulator.h"

namespace physics::app
{
SimulatorModule create_default_simulator_module()
{
    auto local_frame_state = std::make_shared<sim::LocalFrameState>();

    SimulatorModule module;
    module.simulator = std::make_unique<sim::LocalFrameSimulator>(local_frame_state);
    module.renderer = std::make_unique<gfx::LocalFrameGridRenderer>(local_frame_state);
    return module;
}
} // namespace physics::app
