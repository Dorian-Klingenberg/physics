#pragma once

#include <DirectXMath.h>

namespace physics::sim
{
enum class LocalFramePlane
{
    XY,
    XZ,
    YZ,
};

struct LocalFrameState
{
    DirectX::XMFLOAT3 point = {3.0f, 2.0f, 1.5f};
    LocalFramePlane active_plane = LocalFramePlane::XY;
    DirectX::XMFLOAT4X4 view_projection = {};
    DirectX::XMFLOAT4X4 inverse_view_projection = {};
    unsigned int viewport_width = 0;
    unsigned int viewport_height = 0;
    bool picking_ready = false;
    bool show_component_legs = true;
    bool show_lattice_depth = true;

    // Stage 3: free 3D vector mode and orbit camera
    bool free_3d_mode = true;
    float camera_yaw = 0.694f;
    float camera_pitch = 0.507f;
    float camera_distance = 13.4f;
    bool show_box = true;
    bool show_face_diagonals = false;

    // Live camera-screen plane basis — written by renderer each frame
    DirectX::XMFLOAT3 drag_plane_right = {1.0f, 0.0f, 0.0f};
    DirectX::XMFLOAT3 drag_plane_up    = {0.0f, 0.0f, 1.0f};

    // Drag-plane visualization — written by simulator at drag start/end
    bool              is_dragging_point   = false;
    DirectX::XMFLOAT3 drag_plane_center   = {};   // P's world position at click time
    DirectX::XMFLOAT3 drag_plane_vis_right = {1.0f, 0.0f, 0.0f}; // basis frozen at drag start
    DirectX::XMFLOAT3 drag_plane_vis_up    = {0.0f, 0.0f, 1.0f};
};
} // namespace physics::sim
