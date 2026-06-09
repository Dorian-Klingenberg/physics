#include "gfx/local_frame_grid_renderer.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "third_party/imgui/imgui.h"
#include "sim/local_frame_math.h"

using namespace DirectX;

namespace physics::gfx
{
namespace
{
constexpr float k_grid_extent  = 6.0f;
constexpr float k_depth_extent = 4.0f;

// ── Color palette ────────────────────────────────────────────────────────────
const XMFLOAT4 k_grid          = {0.19f, 0.22f, 0.27f, 1.0f};
const XMFLOAT4 k_active_plane  = {0.36f, 0.43f, 0.52f, 1.0f};
const XMFLOAT4 k_depth         = {0.13f, 0.16f, 0.20f, 1.0f};
const XMFLOAT4 k_x             = {0.95f, 0.18f, 0.18f, 1.0f};
const XMFLOAT4 k_y             = {0.22f, 0.86f, 0.35f, 1.0f};
const XMFLOAT4 k_z             = {0.25f, 0.48f, 1.0f,  1.0f};
const XMFLOAT4 k_vector        = {1.0f,  0.86f, 0.24f, 1.0f};
const XMFLOAT4 k_point         = {1.0f,  1.0f,  1.0f,  1.0f};
const XMFLOAT4 k_box           = {0.28f, 0.32f, 0.38f, 0.85f};
const XMFLOAT4 k_face_diag_xy  = {0.90f, 0.85f, 0.30f, 0.80f};
const XMFLOAT4 k_face_diag_xz  = {0.90f, 0.50f, 0.20f, 0.80f};
const XMFLOAT4 k_face_diag_yz  = {0.25f, 0.80f, 0.85f, 0.80f};
const XMFLOAT4 k_theta_arc     = {0.78f, 0.65f, 1.0f,  0.90f};
const XMFLOAT4 k_slice_anchor  = {0.85f, 0.72f, 1.0f,  1.0f};
const XMFLOAT4 k_x_measure     = {0.98f, 0.48f, 0.32f, 1.0f};
const XMFLOAT4 k_y_measure     = {0.52f, 0.95f, 0.58f, 1.0f};
} // namespace

// ── Construction ─────────────────────────────────────────────────────────────

LocalFrameGridRenderer::LocalFrameGridRenderer(std::shared_ptr<sim::LocalFrameState> state)
    : LineRendererBase(4096), state_(std::move(state))
{
    if (!state_)
        throw std::invalid_argument("LocalFrameGridRenderer requires shared state.");
}

const char* LocalFrameGridRenderer::name() const noexcept
{
    return "Stage 3: Free 3D Vector Grid";
}

void LocalFrameGridRenderer::render_ui()
{
    using sim::plane_name;
    ImGui::Text(
        "Line renderer: %s",
        state_->free_3d_mode ? "free 3D" : sim::plane_info(state_->active_plane).name);
    ImGui::Text("Viewport: %u x %u", width_, height_);
    ImGui::Text("Vertices: %zu", vertices_.size());
}

// ── Camera ───────────────────────────────────────────────────────────────────

void LocalFrameGridRenderer::update_camera(const RendererFrameContext& context)
{
    const float aspect = context.height == 0
        ? 1.0f
        : static_cast<float>(context.width) / static_cast<float>(context.height);

    const XMVECTOR target = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    const XMVECTOR up     = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

    XMMATRIX view_projection;
    if (state_->free_3d_mode)
    {
        const float cp   = std::cos(state_->camera_pitch);
        const float dist = state_->camera_distance;
        const XMVECTOR eye = XMVectorSet(
            dist * cp * std::sin(state_->camera_yaw),
            dist * cp * (-std::cos(state_->camera_yaw)),
            dist * std::sin(state_->camera_pitch),
            1.0f);
        const XMMATRIX view       = XMMatrixLookAtLH(eye, target, up);
        const XMMATRIX projection = XMMatrixPerspectiveFovLH(0.75f, aspect, 0.5f, 200.0f);
        view_projection = XMMatrixMultiply(view, projection);

        // Extract the camera's right and up axes directly from the view matrix columns.
        // XMMatrixLookAtLH stores xaxis in column 0 and yaxis in column 1 (row-major layout),
        // so each axis is scattered across rows: right = (_11,_21,_31), up = (_12,_22,_32).
        XMFLOAT4X4 view_mat;
        XMStoreFloat4x4(&view_mat, view);
        state_->drag_plane_right = {view_mat._11, view_mat._21, view_mat._31};
        state_->drag_plane_up    = {view_mat._12, view_mat._22, view_mat._32};
    }
    else
    {
        const XMVECTOR eye        = XMVectorSet(7.5f, -9.0f, 6.5f, 1.0f);
        const XMMATRIX view       = XMMatrixLookAtLH(eye, target, up);
        const XMMATRIX projection = XMMatrixOrthographicLH(13.5f * aspect, 13.5f, 0.1f, 100.0f);
        view_projection = XMMatrixMultiply(view, projection);
    }

    const XMMATRIX inv = XMMatrixInverse(nullptr, view_projection);

    XMStoreFloat4x4(&mapped_constants_->view_projection, view_projection);
    XMStoreFloat4x4(&state_->view_projection, view_projection);
    XMStoreFloat4x4(&state_->inverse_view_projection, inv);
    state_->viewport_width  = context.width;
    state_->viewport_height = context.height;
    state_->picking_ready   = true;
}

// ── Geometry ─────────────────────────────────────────────────────────────────

void LocalFrameGridRenderer::build_vertices()
{
    vertices_.clear();
    vertices_.reserve(512);

    if (state_->free_3d_mode)
        build_free_3d_vertices();
    else
        build_plane_slice_vertices();
}

void LocalFrameGridRenderer::build_free_3d_vertices()
{
    // Ground grid (z = 0, XY plane)
    for (int i = -6; i <= 6; ++i)
    {
        const float p = static_cast<float>(i);
        add_line({-k_grid_extent, p, 0.0f}, {k_grid_extent, p, 0.0f}, k_grid);
        add_line({p, -k_grid_extent, 0.0f}, {p, k_grid_extent, 0.0f}, k_grid);
    }

    // World axes
    add_line({-k_grid_extent - 0.5f, 0.0f, 0.0f}, {k_grid_extent + 0.5f, 0.0f, 0.0f}, k_x);
    add_line({0.0f, -k_grid_extent - 0.5f, 0.0f}, {0.0f, k_grid_extent + 0.5f, 0.0f}, k_y);
    add_line({0.0f, 0.0f, -k_depth_extent - 0.5f}, {0.0f, 0.0f, k_depth_extent + 0.5f}, k_z);

    const float px = state_->point.x;
    const float py = state_->point.y;
    const float pz = state_->point.z;

    // Box: 9 non-staircase edges
    if (state_->show_box)
    {
        add_line({0.0f, 0.0f, 0.0f}, {0.0f, py,   0.0f}, k_box); // bottom face
        add_line({px,   py,   0.0f}, {0.0f, py,   0.0f}, k_box);
        add_line({0.0f, 0.0f, pz  }, {px,   0.0f, pz  }, k_box); // top face
        add_line({px,   0.0f, pz  }, {px,   py,   pz  }, k_box);
        add_line({px,   py,   pz  }, {0.0f, py,   pz  }, k_box);
        add_line({0.0f, py,   pz  }, {0.0f, 0.0f, pz  }, k_box);
        add_line({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, pz  }, k_box); // vertical pillars
        add_line({px,   0.0f, 0.0f}, {px,   0.0f, pz  }, k_box);
        add_line({0.0f, py,   0.0f}, {0.0f, py,   pz  }, k_box);
    }

    // Face diagonals through the origin corner
    if (state_->show_face_diagonals)
    {
        add_line({0.0f, 0.0f, 0.0f}, {px, py, 0.0f}, k_face_diag_xy);
        add_line({0.0f, 0.0f, 0.0f}, {px, 0.0f, pz}, k_face_diag_xz);
        add_line({0.0f, 0.0f, 0.0f}, {0.0f, py, pz}, k_face_diag_yz);
    }

    // Staircase: bright X → Y → Z component legs
    if (state_->show_component_legs)
    {
        add_line({0.0f, 0.0f, 0.0f}, {px,   0.0f, 0.0f}, k_x);
        add_line({px,   0.0f, 0.0f}, {px,   py,   0.0f}, k_y);
        add_line({px,   py,   0.0f}, {px,   py,   pz  }, k_z);
    }

    // Theta arc: polar angle from +Z to P, swept in the vertical plane containing Z and P
    {
        const float pmag = std::sqrt(px*px + py*py + pz*pz);
        const float pxy  = std::sqrt(px*px + py*py);
        if (pmag > 0.1f && pxy > 0.01f)
        {
            constexpr float arc_r = 1.5f;
            constexpr int   arc_n = 24;
            const float pxh   = px / pxy;
            const float pyh   = py / pxy;
            const float theta = std::acos(std::clamp(pz / pmag, -1.0f, 1.0f));
            XMFLOAT3 prev = {0.0f, 0.0f, arc_r};
            for (int i = 1; i <= arc_n; ++i)
            {
                const float t = theta * static_cast<float>(i) / static_cast<float>(arc_n);
                const XMFLOAT3 cur = {arc_r * std::sin(t) * pxh,
                                      arc_r * std::sin(t) * pyh,
                                      arc_r * std::cos(t)};
                add_line(prev, cur, k_theta_arc);
                prev = cur;
            }
        }
    }

    // Vector P and its crosshair
    add_line({0.0f, 0.0f, 0.0f}, {px, py, pz}, k_vector);
    constexpr float m = 0.18f;
    add_line({px - m, py,     pz    }, {px + m, py,     pz    }, k_point);
    add_line({px,     py - m, pz    }, {px,     py + m, pz    }, k_point);
    add_line({px,     py,     pz - m}, {px,     py,     pz + m}, k_point);
}

void LocalFrameGridRenderer::build_plane_slice_vertices()
{
    using namespace sim;
    const float locked = locked_component(state_->point, state_->active_plane);

    // Active-plane grid
    for (int i = -6; i <= 6; ++i)
    {
        const float p = static_cast<float>(i);
        add_line(
            point_on_plane(state_->active_plane, -k_grid_extent, p, locked),
            point_on_plane(state_->active_plane,  k_grid_extent, p, locked), k_grid);
        add_line(
            point_on_plane(state_->active_plane, p, -k_grid_extent, locked),
            point_on_plane(state_->active_plane, p,  k_grid_extent, locked), k_grid);
    }

    // Active-plane outline
    add_line(point_on_plane(state_->active_plane, -k_grid_extent, -k_grid_extent, locked),
             point_on_plane(state_->active_plane,  k_grid_extent, -k_grid_extent, locked), k_active_plane);
    add_line(point_on_plane(state_->active_plane,  k_grid_extent, -k_grid_extent, locked),
             point_on_plane(state_->active_plane,  k_grid_extent,  k_grid_extent, locked), k_active_plane);
    add_line(point_on_plane(state_->active_plane,  k_grid_extent,  k_grid_extent, locked),
             point_on_plane(state_->active_plane, -k_grid_extent,  k_grid_extent, locked), k_active_plane);
    add_line(point_on_plane(state_->active_plane, -k_grid_extent,  k_grid_extent, locked),
             point_on_plane(state_->active_plane, -k_grid_extent, -k_grid_extent, locked), k_active_plane);

    // Depth hints (back wall)
    if (state_->show_lattice_depth)
    {
        for (int i = -6; i <= 6; i += 3)
        {
            const float p = static_cast<float>(i);
            add_line({p, k_grid_extent, -k_depth_extent}, {p, k_grid_extent, k_depth_extent}, k_depth);
            add_line({-k_grid_extent, p, -k_depth_extent}, {-k_grid_extent, p, k_depth_extent}, k_depth);
        }
        add_line({-k_grid_extent, k_grid_extent,  k_depth_extent}, {k_grid_extent, k_grid_extent,  k_depth_extent}, k_depth);
        add_line({-k_grid_extent, -k_grid_extent, k_depth_extent}, {-k_grid_extent, k_grid_extent, k_depth_extent}, k_depth);
        add_line({-k_grid_extent, k_grid_extent,  -k_depth_extent}, {k_grid_extent, k_grid_extent,  -k_depth_extent}, k_depth);
        add_line({-k_grid_extent, -k_grid_extent, -k_depth_extent}, {-k_grid_extent, k_grid_extent, -k_depth_extent}, k_depth);
    }

    // World axes
    add_line({-k_grid_extent - 0.5f, 0.0f, 0.0f}, {k_grid_extent + 0.5f, 0.0f, 0.0f}, k_x);
    add_line({0.0f, -k_grid_extent - 0.5f, 0.0f}, {0.0f, k_grid_extent + 0.5f, 0.0f}, k_y);
    add_line({0.0f, 0.0f, -k_depth_extent - 0.5f}, {0.0f, 0.0f, k_depth_extent + 0.5f}, k_z);

    const XMFLOAT3 pt     = state_->point;
    const XMFLOAT3 anchor = point_on_plane(state_->active_plane, 0.0f, 0.0f, locked);

    // Component legs
    if (state_->show_component_legs)
    {
        const float first  = first_component(pt, state_->active_plane);
        const float second = second_component(pt, state_->active_plane);
        add_line(anchor,
                 point_on_plane(state_->active_plane, first,  0.0f,   locked), k_x);
        add_line(point_on_plane(state_->active_plane, first,  0.0f,   locked), pt, k_y_measure);
        add_line(point_on_plane(state_->active_plane, 0.0f,   second, locked), pt, k_x_measure);
    }

    // Slice-anchor marker and connector
    constexpr float am = 0.25f;
    add_line(point_on_plane(state_->active_plane, -am, -am, locked),
             point_on_plane(state_->active_plane,  am,  am, locked), k_slice_anchor);
    add_line(point_on_plane(state_->active_plane, -am,  am, locked),
             point_on_plane(state_->active_plane,  am, -am, locked), k_slice_anchor);
    add_line({0.0f, 0.0f, 0.0f}, anchor, k_slice_anchor);

    // Vector P and crosshair
    add_line({0.0f, 0.0f, 0.0f}, pt, k_vector);
    constexpr float m = 0.18f;
    add_line({pt.x - m, pt.y,     pt.z    }, {pt.x + m, pt.y,     pt.z    }, k_point);
    add_line({pt.x,     pt.y - m, pt.z    }, {pt.x,     pt.y + m, pt.z    }, k_point);
    add_line({pt.x,     pt.y,     pt.z - m}, {pt.x,     pt.y,     pt.z + m}, k_point);
}

} // namespace physics::gfx
