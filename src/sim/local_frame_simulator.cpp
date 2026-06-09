#include "sim/local_frame_simulator.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>

#include "third_party/imgui/imgui.h"
#include "sim/local_frame_math.h"

namespace physics::sim
{
namespace
{
constexpr float k_grid_extent = 6.0f;
} // namespace

// ── Construction ─────────────────────────────────────────────────────────────

LocalFrameSimulator::LocalFrameSimulator(std::shared_ptr<LocalFrameState> state)
    : state_(std::move(state))
{
    if (!state_)
        throw std::invalid_argument("LocalFrameSimulator requires shared state.");
}

// ── Lesson interface ──────────────────────────────────────────────────────────

const char* LocalFrameSimulator::name() const noexcept
{
    return "Local Frame Lab - Stage 3";
}

void LocalFrameSimulator::update(const SimulationTick& tick)
{
    elapsed_seconds_ = tick.total_seconds;
}

void LocalFrameSimulator::render_ui(const SimulatorFrameContext& context)
{
    handle_interaction();
    draw_scene_labels();

    ImGui::SetNextWindowPos(ImVec2(24.0f, 24.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(460.0f, 400.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin(name());

    ImGui::Checkbox("Free 3D mode", &state_->free_3d_mode);

    if (state_->free_3d_mode)
        render_free_3d_ui(context);
    else
        render_plane_slice_ui();

    ImGui::Separator();
    ImGui::Text("Client area: %u x %u", context.client_width, context.client_height);
    ImGui::Text("Elapsed: %.2f seconds", elapsed_seconds_);
    ImGui::End();
}

void LocalFrameSimulator::render_free_3d_ui(const SimulatorFrameContext& /*context*/)
{
    ImGui::TextUnformatted("Left-drag P to move it.  Right-drag to orbit.  Scroll to zoom.");
    ImGui::Separator();

    ImGui::SliderFloat("P.x", &state_->point.x, -k_grid_extent, k_grid_extent, "%.2f");
    ImGui::SliderFloat("P.y", &state_->point.y, -k_grid_extent, k_grid_extent, "%.2f");
    ImGui::SliderFloat("P.z", &state_->point.z, -k_grid_extent, k_grid_extent, "%.2f");

    if (ImGui::Button("Reset P")) reset_point();
    ImGui::SameLine(); ImGui::Checkbox("Legs",       &state_->show_component_legs);
    ImGui::SameLine(); ImGui::Checkbox("Box",        &state_->show_box);
    ImGui::SameLine(); ImGui::Checkbox("Face diags", &state_->show_face_diagonals);

    ImGui::Separator();

    const float mag = magnitude(state_->point);
    ImGui::Text("P = (%.2f, %.2f, %.2f)", state_->point.x, state_->point.y, state_->point.z);
    ImGui::Text("|P| = %.3f", mag);

    if (mag > 0.0001f)
    {
        constexpr float r2d = 180.0f / 3.14159265f;
        const float alpha = std::acos(std::clamp(state_->point.x / mag, -1.0f, 1.0f));
        const float beta  = std::acos(std::clamp(state_->point.y / mag, -1.0f, 1.0f));
        const float gamma = std::acos(std::clamp(state_->point.z / mag, -1.0f, 1.0f));
        ImGui::Text("alpha (with X) = %.1f deg", alpha * r2d);
        ImGui::Text("beta  (with Y) = %.1f deg", beta  * r2d);
        ImGui::Text("gamma (with Z) = %.1f deg", gamma * r2d);
        ImGui::Text("cos^2+cos^2+cos^2 = %.4f",
            (state_->point.x * state_->point.x +
             state_->point.y * state_->point.y +
             state_->point.z * state_->point.z) / (mag * mag));
    }

    ImGui::Separator();
    ImGui::SliderFloat("Yaw",      &state_->camera_yaw,      -3.14f, 3.14f, "%.2f rad");
    ImGui::SliderFloat("Pitch",    &state_->camera_pitch,    0.05f,  1.50f, "%.2f rad");
    ImGui::SliderFloat("Distance", &state_->camera_distance, 3.0f,  30.0f, "%.1f");
}

void LocalFrameSimulator::render_plane_slice_ui()
{
    ImGui::TextUnformatted("Plane-slice interaction inside a 3D lattice.");
    ImGui::Separator();

    int plane_index = static_cast<int>(state_->active_plane);
    if (ImGui::RadioButton("XY", plane_index == static_cast<int>(LocalFramePlane::XY)))
        set_active_plane(LocalFramePlane::XY);
    ImGui::SameLine();
    if (ImGui::RadioButton("XZ", plane_index == static_cast<int>(LocalFramePlane::XZ)))
        set_active_plane(LocalFramePlane::XZ);
    ImGui::SameLine();
    if (ImGui::RadioButton("YZ", plane_index == static_cast<int>(LocalFramePlane::YZ)))
        set_active_plane(LocalFramePlane::YZ);

    const PlaneInfo info = plane_info(state_->active_plane);
    float comps[2] = {
        first_component(state_->point, state_->active_plane),
        second_component(state_->point, state_->active_plane),
    };
    char slider_label[64] = {};
    std::snprintf(slider_label, sizeof(slider_label), "P.%s / P.%s",
        info.first_label, info.second_label);
    if (ImGui::SliderFloat2(slider_label, comps, -k_grid_extent, k_grid_extent, "%.2f"))
        set_point_from_active_components(comps[0], comps[1]);

    if (ImGui::Button("Reset P")) reset_point();
    ImGui::SameLine(); ImGui::Checkbox("Component legs", &state_->show_component_legs);
    ImGui::Checkbox("Depth hints", &state_->show_lattice_depth);

    ImGui::Separator();
    ImGui::Text("P = (%.2f, %.2f, %.2f)", state_->point.x, state_->point.y, state_->point.z);
    ImGui::Text("|P| = %.3f", magnitude(state_->point));
    ImGui::Text("Distance along %s: |%s| = %.3f",
        info.first_label,  info.first_label,  std::abs(comps[0]));
    ImGui::Text("Distance along %s: |%s| = %.3f",
        info.second_label, info.second_label, std::abs(comps[1]));
    ImGui::Text("Active slice: %s, preserving %s = %.2f",
        info.name, info.locked_label,
        locked_component(state_->point, state_->active_plane));
    ImGui::Text("Click or drag the scene background to move P on the %s plane.", info.name);
}

// ── Scene overlays ────────────────────────────────────────────────────────────

void LocalFrameSimulator::draw_scene_labels() const
{
    if (!state_->picking_ready || state_->viewport_width == 0 || state_->viewport_height == 0)
        return;

    if (state_->free_3d_mode)
    {
        // Drag-plane overlay — only while dragging.
        // Drawn entirely in 2D screen space so it is guaranteed parallel to the screen.
        if (state_->is_dragging_point)
        {
            const float cx = state_->drag_plane_center.x;
            const float cy = state_->drag_plane_center.y;
            const float cz = state_->drag_plane_center.z;
            const DirectX::XMFLOAT3& r = state_->drag_plane_vis_right;
            const DirectX::XMFLOAT3& u = state_->drag_plane_vis_up;
            constexpr float ext = k_grid_extent;

            const auto world_corner = [&](const float rs, const float us) -> DirectX::XMFLOAT3
            {
                return {cx + r.x * rs + u.x * us,
                        cy + r.y * rs + u.y * us,
                        cz + r.z * rs + u.z * us};
            };

            // Project without NDC clipping — corners can be off-screen and ImGui clips them.
            // Only reject corners behind the camera (w <= 0).
            const auto project = [&](const DirectX::XMFLOAT3 world, ImVec2& out) -> bool
            {
                using namespace DirectX;
                const XMMATRIX vp   = XMLoadFloat4x4(&state_->view_projection);
                const XMVECTOR clip = XMVector4Transform(
                    XMVectorSet(world.x, world.y, world.z, 1.0f), vp);
                XMFLOAT4 c;
                XMStoreFloat4(&c, clip);
                if (c.w <= 0.0f) return false;
                out.x = (c.x / c.w * 0.5f + 0.5f) * static_cast<float>(state_->viewport_width);
                out.y = (0.5f - c.y / c.w * 0.5f) * static_cast<float>(state_->viewport_height);
                return true;
            };

            // sc[0]=(-ext,-ext)  sc[1]=(+ext,-ext)  sc[2]=(+ext,+ext)  sc[3]=(-ext,+ext)
            ImVec2 sc[4] = {};
            bool all_on = true;
            const DirectX::XMFLOAT3 wc[4] = {
                world_corner(-ext, -ext), world_corner(ext, -ext),
                world_corner( ext,  ext), world_corner(-ext,  ext),
            };
            for (int i = 0; i < 4; ++i)
                if (!project(wc[i], sc[i]))
                    all_on = false;

            if (all_on)
            {
                auto* bg = ImGui::GetBackgroundDrawList();

                bg->AddConvexPolyFilled(sc, 4, IM_COL32(100, 140, 200, 18));

                // Grid lines interpolated in screen space (2-unit spacing = 7 lines each axis).
                constexpr ImU32 k_line = IM_COL32(97, 122, 166, 102);
                for (int i = -6; i <= 6; i += 2)
                {
                    const float t = (static_cast<float>(i) + ext) / (2.0f * ext);

                    // Lines parallel to the right axis (constant up position)
                    // Left edge: sc[0]→sc[3],  right edge: sc[1]→sc[2]
                    const ImVec2 ll = {sc[0].x + (sc[3].x - sc[0].x) * t,
                                       sc[0].y + (sc[3].y - sc[0].y) * t};
                    const ImVec2 lr = {sc[1].x + (sc[2].x - sc[1].x) * t,
                                       sc[1].y + (sc[2].y - sc[1].y) * t};
                    bg->AddLine(ll, lr, k_line, 1.0f);

                    // Lines parallel to the up axis (constant right position)
                    // Bottom edge: sc[0]→sc[1],  top edge: sc[3]→sc[2]
                    const ImVec2 lb = {sc[0].x + (sc[1].x - sc[0].x) * t,
                                       sc[0].y + (sc[1].y - sc[0].y) * t};
                    const ImVec2 lt = {sc[3].x + (sc[2].x - sc[3].x) * t,
                                       sc[3].y + (sc[2].y - sc[3].y) * t};
                    bg->AddLine(lb, lt, k_line, 1.0f);
                }

                constexpr ImU32 k_border = IM_COL32(140, 175, 225, 204);
                bg->AddLine(sc[0], sc[1], k_border, 1.5f);
                bg->AddLine(sc[1], sc[2], k_border, 1.5f);
                bg->AddLine(sc[2], sc[3], k_border, 1.5f);
                bg->AddLine(sc[3], sc[0], k_border, 1.5f);
            }
        }
    }
    else
    {
        draw_active_plane_overlay();
    }

    auto* dl = ImGui::GetBackgroundDrawList();
    const auto add_text = [dl](const ImVec2 pos, const ImU32 color, const char* text)
    {
        dl->AddText(ImVec2(pos.x + 1.0f, pos.y + 1.0f), IM_COL32(0, 0, 0, 210), text);
        dl->AddText(pos, color, text);
    };

    // P label
    ImVec2 p_screen = {};
    if (world_to_screen(state_->point, p_screen))
    {
        char label[96] = {};
        std::snprintf(label, sizeof(label),
            "P  x=%.2f  y=%.2f  z=%.2f",
            state_->point.x, state_->point.y, state_->point.z);
        add_text(ImVec2(p_screen.x + 10.0f, p_screen.y - 20.0f),
            IM_COL32(255, 245, 190, 255), label);
    }

    // Component leg labels
    if (state_->show_component_legs)
    {
        if (state_->free_3d_mode)
        {
            const float x = state_->point.x;
            const float y = state_->point.y;
            const float z = state_->point.z;

            const auto leg_label = [&](DirectX::XMFLOAT3 mid, ImU32 color, const char* axis, float val)
            {
                ImVec2 s = {};
                if (world_to_screen(mid, s))
                {
                    char buf[48] = {};
                    std::snprintf(buf, sizeof(buf), "|%s| = %.2f", axis, std::abs(val));
                    add_text(ImVec2(s.x + 8.0f, s.y - 8.0f), color, buf);
                }
            };

            leg_label({x * 0.5f, 0.0f,     0.0f    }, IM_COL32(245,  80,  80, 255), "x", x);
            leg_label({x,        y * 0.5f,  0.0f    }, IM_COL32( 80, 235, 110, 255), "y", y);
            leg_label({x,        y,         z * 0.5f}, IM_COL32(100, 150, 255, 255), "z", z);
        }
        else
        {
            const PlaneInfo info   = plane_info(state_->active_plane);
            const float first      = first_component(state_->point, state_->active_plane);
            const float second     = second_component(state_->point, state_->active_plane);
            const float locked     = locked_component(state_->point, state_->active_plane);

            ImVec2 s2 = {};
            if (world_to_screen(point_on_plane(state_->active_plane, first, second * 0.5f, locked), s2))
            {
                char buf[48] = {};
                std::snprintf(buf, sizeof(buf), "|%s| = %.2f", info.second_label, std::abs(second));
                add_text(ImVec2(s2.x + 8.0f, s2.y - 8.0f), IM_COL32(135, 245, 155, 255), buf);
            }
            ImVec2 s1 = {};
            if (world_to_screen(point_on_plane(state_->active_plane, first * 0.5f, second, locked), s1))
            {
                char buf[48] = {};
                std::snprintf(buf, sizeof(buf), "|%s| = %.2f", info.first_label, std::abs(first));
                add_text(ImVec2(s1.x + 8.0f, s1.y - 8.0f), IM_COL32(255, 150, 105, 255), buf);
            }
        }
    }

    // Theta label — at the midpoint of the polar-angle arc
    if (state_->free_3d_mode)
    {
        const float x = state_->point.x;
        const float y = state_->point.y;
        const float z = state_->point.z;
        const float pmag = magnitude(state_->point);
        const float pxy  = std::sqrt(x*x + y*y);
        if (pmag > 0.1f && pxy > 0.01f)
        {
            constexpr float r2d   = 180.0f / 3.14159265f;
            constexpr float arc_r = 1.5f;
            const float theta = std::acos(std::clamp(z / pmag, -1.0f, 1.0f));
            const float tmid  = theta * 0.5f;
            const DirectX::XMFLOAT3 arc_mid = {
                arc_r * std::sin(tmid) * (x / pxy),
                arc_r * std::sin(tmid) * (y / pxy),
                arc_r * std::cos(tmid),
            };
            ImVec2 s = {};
            if (world_to_screen(arc_mid, s))
            {
                char buf[64] = {};
                std::snprintf(buf, sizeof(buf),
                    "theta = arccos(z/|P|) = %.1f deg", theta * r2d);
                add_text(ImVec2(s.x + 10.0f, s.y - 8.0f), IM_COL32(198, 165, 255, 255), buf);
            }
        }
    }

    // Face diagonal labels (free 3D only)
    if (state_->free_3d_mode && state_->show_face_diagonals)
    {
        const float x = state_->point.x;
        const float y = state_->point.y;
        const float z = state_->point.z;

        const auto diag_label = [&](DirectX::XMFLOAT3 mid, ImU32 color, const char* formula, float val)
        {
            ImVec2 s = {};
            if (world_to_screen(mid, s))
            {
                char buf[64] = {};
                std::snprintf(buf, sizeof(buf), "%s = %.2f", formula, val);
                add_text(ImVec2(s.x + 8.0f, s.y - 8.0f), color, buf);
            }
        };

        diag_label({x * 0.5f, y * 0.5f, 0.0f    },
            IM_COL32(230, 215,  76, 255),
            "sqrt(x2+y2)", std::sqrt(x*x + y*y));
        diag_label({x * 0.5f, 0.0f,     z * 0.5f},
            IM_COL32(230, 128,  51, 255),
            "sqrt(x2+z2)", std::sqrt(x*x + z*z));
        diag_label({0.0f,     y * 0.5f, z * 0.5f},
            IM_COL32( 64, 204, 217, 255),
            "sqrt(y2+z2)", std::sqrt(y*y + z*z));
    }

    // Plane-mode: locked-axis intersection label
    if (!state_->free_3d_mode)
    {
        const PlaneInfo info   = plane_info(state_->active_plane);
        const float locked     = locked_component(state_->point, state_->active_plane);
        ImVec2 anchor_screen   = {};
        if (world_to_screen(point_on_plane(state_->active_plane, 0.0f, 0.0f, locked), anchor_screen))
        {
            char buf[80] = {};
            std::snprintf(buf, sizeof(buf), "%s intersects %s",
                locked_axis_name(state_->active_plane), info.name);
            add_text(ImVec2(anchor_screen.x + 10.0f, anchor_screen.y + 10.0f),
                IM_COL32(215, 190, 255, 255), buf);
        }
    }

    // Axis end labels
    struct AxisLabel { DirectX::XMFLOAT3 world; const char* text; ImU32 color; };
    const AxisLabel axes[] = {
        {{6.7f, 0.0f, 0.0f}, "X", IM_COL32(245,  70,  70, 255)},
        {{0.0f, 6.7f, 0.0f}, "Y", IM_COL32( 80, 235, 110, 255)},
        {{0.0f, 0.0f, 4.7f}, "Z", IM_COL32(100, 150, 255, 255)},
    };
    for (const AxisLabel& a : axes)
    {
        ImVec2 s = {};
        if (world_to_screen(a.world, s))
            add_text(ImVec2(s.x + 4.0f, s.y - 4.0f), a.color, a.text);
    }
}

void LocalFrameSimulator::draw_active_plane_overlay() const
{
    const float locked = locked_component(state_->point, state_->active_plane);
    const DirectX::XMFLOAT3 corners_world[] = {
        point_on_plane(state_->active_plane, -k_grid_extent, -k_grid_extent, locked),
        point_on_plane(state_->active_plane,  k_grid_extent, -k_grid_extent, locked),
        point_on_plane(state_->active_plane,  k_grid_extent,  k_grid_extent, locked),
        point_on_plane(state_->active_plane, -k_grid_extent,  k_grid_extent, locked),
    };

    ImVec2 corners_screen[4] = {};
    for (int i = 0; i < 4; ++i)
        if (!world_to_screen(corners_world[i], corners_screen[i]))
            return;

    ImGui::GetBackgroundDrawList()->AddConvexPolyFilled(corners_screen, 4, IM_COL32(95, 135, 180, 22));
}

// ── Interaction ───────────────────────────────────────────────────────────────

void LocalFrameSimulator::handle_interaction()
{
    const ImGuiIO& io = ImGui::GetIO();
    if (!state_->picking_ready)
        return;

    if (state_->free_3d_mode)
    {
        // Right drag: orbit
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)) orbiting_ = false;
        if (!orbiting_ && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !io.WantCaptureMouse)
            orbiting_ = true;
        if (orbiting_ && ImGui::IsMouseDown(ImGuiMouseButton_Right))
        {
            state_->camera_yaw -= io.MouseDelta.x * 0.005f;
            state_->camera_pitch = std::clamp(
                state_->camera_pitch + io.MouseDelta.y * 0.005f, 0.05f, 1.50f);
        }

        // Scroll: zoom
        if (!io.WantCaptureMouse)
            state_->camera_distance = std::clamp(
                state_->camera_distance - io.MouseWheel * 0.8f, 3.0f, 30.0f);

        // Left click anywhere: place P on the camera-screen plane through its current
        // position, then keep dragging on that same plane until mouse release.
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            dragging_point_          = false;
            state_->is_dragging_point = false;
        }
        if (!dragging_point_ && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !io.WantCaptureMouse)
        {
            dragging_point_    = true;
            drag_plane_origin_ = state_->point;
            drag_plane_normal_ = compute_camera_look_dir();
            state_->is_dragging_point    = true;
            state_->drag_plane_vis_right = state_->drag_plane_right;
            state_->drag_plane_vis_up    = state_->drag_plane_up;

            // Grid center: project the world origin (0,0,0) onto the drag plane along the
            // look direction.  Line = t*n through origin; plane passes through P, normal n.
            // t = dot(n, P)  →  center = t * n
            const DirectX::XMFLOAT3& n = drag_plane_normal_;
            const DirectX::XMFLOAT3& p = drag_plane_origin_;
            const float t = n.x * p.x + n.y * p.y + n.z * p.z;
            state_->drag_plane_center = {n.x * t, n.y * t, n.z * t};

            move_point_on_drag_plane();
        }
        if (dragging_point_ && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            move_point_on_drag_plane();
    }
    else
    {
        // Plane mode: left drag moves P on the active coordinate plane
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) dragging_point_ = false;
        if (!dragging_point_ && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !io.WantCaptureMouse)
            dragging_point_ = true;
        if (dragging_point_ && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            move_point_to_mouse();
    }
}

void LocalFrameSimulator::move_point_on_drag_plane()
{
    const ImGuiIO& io = ImGui::GetIO();
    if (state_->viewport_width == 0 || state_->viewport_height == 0) return;

    const float ndc_x = (2.0f * io.MousePos.x / static_cast<float>(state_->viewport_width))  - 1.0f;
    const float ndc_y =  1.0f - (2.0f * io.MousePos.y / static_cast<float>(state_->viewport_height));

    using namespace DirectX;
    const XMMATRIX inv = XMLoadFloat4x4(&state_->inverse_view_projection);
    const XMVECTOR near_pt = XMVector3TransformCoord(XMVectorSet(ndc_x, ndc_y, 0.0f, 1.0f), inv);
    const XMVECTOR far_pt  = XMVector3TransformCoord(XMVectorSet(ndc_x, ndc_y, 1.0f, 1.0f), inv);
    const XMVECTOR dir     = XMVectorSubtract(far_pt, near_pt);

    XMFLOAT3 near_world = {};
    XMFLOAT3 ray_dir    = {};
    XMStoreFloat3(&near_world, near_pt);
    XMStoreFloat3(&ray_dir, dir);

    const XMFLOAT3& n = drag_plane_normal_;
    const XMFLOAT3& o = drag_plane_origin_;
    const float denom = n.x * ray_dir.x + n.y * ray_dir.y + n.z * ray_dir.z;
    if (std::abs(denom) < 0.0001f) return;

    const float t = (n.x * (o.x - near_world.x) +
                     n.y * (o.y - near_world.y) +
                     n.z * (o.z - near_world.z)) / denom;

    state_->point = {
        std::clamp(near_world.x + ray_dir.x * t, -k_grid_extent, k_grid_extent),
        std::clamp(near_world.y + ray_dir.y * t, -k_grid_extent, k_grid_extent),
        std::clamp(near_world.z + ray_dir.z * t, -k_grid_extent, k_grid_extent),
    };
}

void LocalFrameSimulator::move_point_to_mouse()
{
    const ImGuiIO& io = ImGui::GetIO();
    if (state_->viewport_width == 0 || state_->viewport_height == 0) return;

    const float ndc_x = (2.0f * io.MousePos.x / static_cast<float>(state_->viewport_width))  - 1.0f;
    const float ndc_y =  1.0f - (2.0f * io.MousePos.y / static_cast<float>(state_->viewport_height));

    using namespace DirectX;
    const XMMATRIX inv = XMLoadFloat4x4(&state_->inverse_view_projection);
    const XMVECTOR near_pt = XMVector3TransformCoord(XMVectorSet(ndc_x, ndc_y, 0.0f, 1.0f), inv);
    const XMVECTOR far_pt  = XMVector3TransformCoord(XMVectorSet(ndc_x, ndc_y, 1.0f, 1.0f), inv);
    const XMVECTOR dir     = XMVectorSubtract(far_pt, near_pt);

    XMFLOAT3 near_world = {};
    XMFLOAT3 ray_dir    = {};
    XMStoreFloat3(&near_world, near_pt);
    XMStoreFloat3(&ray_dir, dir);

    float near_axis = 0.0f;
    float dir_axis  = 0.0f;
    switch (state_->active_plane)
    {
    case LocalFramePlane::XY: near_axis = near_world.z; dir_axis = ray_dir.z; break;
    case LocalFramePlane::XZ: near_axis = near_world.y; dir_axis = ray_dir.y; break;
    case LocalFramePlane::YZ: near_axis = near_world.x; dir_axis = ray_dir.x; break;
    }
    if (std::abs(dir_axis) < 0.0001f) return;

    const float locked = locked_component(state_->point, state_->active_plane);
    const float t      = (locked - near_axis) / dir_axis;
    const float x      = near_world.x + ray_dir.x * t;
    const float y      = near_world.y + ray_dir.y * t;
    const float z      = near_world.z + ray_dir.z * t;

    switch (state_->active_plane)
    {
    case LocalFramePlane::XY:
        state_->point = {std::clamp(x, -k_grid_extent, k_grid_extent),
                         std::clamp(y, -k_grid_extent, k_grid_extent), locked}; break;
    case LocalFramePlane::XZ:
        state_->point = {std::clamp(x, -k_grid_extent, k_grid_extent), locked,
                         std::clamp(z, -k_grid_extent, k_grid_extent)}; break;
    case LocalFramePlane::YZ:
        state_->point = {locked,
                         std::clamp(y, -k_grid_extent, k_grid_extent),
                         std::clamp(z, -k_grid_extent, k_grid_extent)}; break;
    }
}

// ── Utilities ─────────────────────────────────────────────────────────────────

DirectX::XMFLOAT3 LocalFrameSimulator::compute_camera_look_dir() const
{
    using namespace DirectX;
    const float cp   = std::cos(state_->camera_pitch);
    const float dist = state_->camera_distance;
    const float ex   = dist * cp * std::sin(state_->camera_yaw);
    const float ey   = dist * cp * (-std::cos(state_->camera_yaw));
    const float ez   = dist * std::sin(state_->camera_pitch);
    const XMVECTOR look = XMVector3Normalize(XMVectorSet(-ex, -ey, -ez, 0.0f));
    XMFLOAT3 result = {};
    XMStoreFloat3(&result, look);
    return result;
}

bool LocalFrameSimulator::world_to_screen(
    const DirectX::XMFLOAT3 world,
    ImVec2& screen) const
{
    using namespace DirectX;
    const XMMATRIX vp       = XMLoadFloat4x4(&state_->view_projection);
    const XMVECTOR projected = XMVector3TransformCoord(
        XMVectorSet(world.x, world.y, world.z, 1.0f), vp);

    XMFLOAT3 ndc = {};
    XMStoreFloat3(&ndc, projected);
    if (ndc.x < -1.2f || ndc.x > 1.2f || ndc.y < -1.2f || ndc.y > 1.2f)
        return false;

    screen.x = (ndc.x * 0.5f + 0.5f) * static_cast<float>(state_->viewport_width);
    screen.y = (0.5f - ndc.y * 0.5f)  * static_cast<float>(state_->viewport_height);
    return true;
}

void LocalFrameSimulator::set_active_plane(const LocalFramePlane plane)
{
    if (state_->active_plane == plane) return;
    state_->active_plane = plane;
    dragging_point_      = false;
}

void LocalFrameSimulator::set_point_from_active_components(const float first, const float second)
{
    state_->point = point_on_plane(
        state_->active_plane,
        std::clamp(first,  -k_grid_extent, k_grid_extent),
        std::clamp(second, -k_grid_extent, k_grid_extent),
        locked_component(state_->point, state_->active_plane));
}

void LocalFrameSimulator::reset_point()
{
    state_->point = state_->free_3d_mode
        ? DirectX::XMFLOAT3{3.0f, 2.0f, 1.5f}
        : DirectX::XMFLOAT3{3.0f, 2.0f, 0.0f};
}

} // namespace physics::sim
