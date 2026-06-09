#pragma once

#include <cmath>

#include <DirectXMath.h>

#include "sim/local_frame_state.h"

// Shared math helpers for LocalFrameSimulator and LocalFrameGridRenderer.
// All functions are inline; include this header in both translation units.

namespace physics::sim
{

struct PlaneInfo
{
    const char* name        = "XY";
    const char* first_label = "x";
    const char* second_label = "y";
    const char* locked_label = "z";
};

inline float magnitude(const DirectX::XMFLOAT3& p)
{
    return std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
}

inline const char* plane_name(const LocalFramePlane plane)
{
    switch (plane)
    {
    case LocalFramePlane::XY: return "XY";
    case LocalFramePlane::XZ: return "XZ";
    case LocalFramePlane::YZ: return "YZ";
    default:                  return "Unknown";
    }
}

inline PlaneInfo plane_info(const LocalFramePlane plane)
{
    switch (plane)
    {
    case LocalFramePlane::XY: return {"XY", "x", "y", "z"};
    case LocalFramePlane::XZ: return {"XZ", "x", "z", "y"};
    case LocalFramePlane::YZ: return {"YZ", "y", "z", "x"};
    default:                  return {};
    }
}

inline const char* locked_axis_name(const LocalFramePlane plane)
{
    switch (plane)
    {
    case LocalFramePlane::XY: return "Z-axis";
    case LocalFramePlane::XZ: return "Y-axis";
    case LocalFramePlane::YZ: return "X-axis";
    default:                  return "axis";
    }
}

inline float first_component(const DirectX::XMFLOAT3& p, const LocalFramePlane plane)
{
    switch (plane)
    {
    case LocalFramePlane::XY:
    case LocalFramePlane::XZ: return p.x;
    case LocalFramePlane::YZ: return p.y;
    default:                  return 0.0f;
    }
}

inline float second_component(const DirectX::XMFLOAT3& p, const LocalFramePlane plane)
{
    switch (plane)
    {
    case LocalFramePlane::XY: return p.y;
    case LocalFramePlane::XZ:
    case LocalFramePlane::YZ: return p.z;
    default:                  return 0.0f;
    }
}

inline float locked_component(const DirectX::XMFLOAT3& p, const LocalFramePlane plane)
{
    switch (plane)
    {
    case LocalFramePlane::XY: return p.z;
    case LocalFramePlane::XZ: return p.y;
    case LocalFramePlane::YZ: return p.x;
    default:                  return 0.0f;
    }
}

inline DirectX::XMFLOAT3 point_on_plane(
    const LocalFramePlane plane,
    const float first,
    const float second,
    const float locked)
{
    switch (plane)
    {
    case LocalFramePlane::XY: return {first,  second, locked};
    case LocalFramePlane::XZ: return {first,  locked, second};
    case LocalFramePlane::YZ: return {locked, first,  second};
    default:                  return {};
    }
}

} // namespace physics::sim
