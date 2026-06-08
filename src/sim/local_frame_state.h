#pragma once

#include <DirectXMath.h>

namespace physics::sim
{
struct LocalFrameState
{
    DirectX::XMFLOAT3 point = {3.0f, 2.0f, 0.0f};
    DirectX::XMFLOAT4X4 view_projection = {};
    DirectX::XMFLOAT4X4 inverse_view_projection = {};
    unsigned int viewport_width = 0;
    unsigned int viewport_height = 0;
    bool picking_ready = false;
    bool show_component_legs = true;
    bool show_lattice_depth = true;
};
} // namespace physics::sim
