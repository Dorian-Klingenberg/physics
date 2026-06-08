#pragma once

namespace physics::sim
{
struct SimulatorFrameContext
{
    unsigned int client_width = 0;
    unsigned int client_height = 0;
    unsigned int frame_index = 0;
    float imgui_display_width = 0.0f;
    float imgui_display_height = 0.0f;
};
} // namespace physics::sim
