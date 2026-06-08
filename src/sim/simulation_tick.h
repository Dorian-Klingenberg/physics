#pragma once

#include <cstdint>

namespace physics::sim
{
struct SimulationTick
{
    float delta_seconds = 0.0f;
    float total_seconds = 0.0f;
    std::uint64_t frame_number = 0;
};
} // namespace physics::sim
