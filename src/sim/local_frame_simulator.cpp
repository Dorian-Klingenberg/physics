#include "sim/local_frame_simulator.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>

#include "third_party/imgui/imgui.h"

namespace physics::sim
{
namespace
{
constexpr float k_grid_extent = 6.0f;

struct PlaneInfo
{
    const char* name = "XY";
    const char* first_label = "x";
    const char* second_label = "y";
    const char* locked_label = "z";
};

float magnitude(const DirectX::XMFLOAT3& point)
{
    return std::sqrt(
        point.x * point.x +
        point.y * point.y +
        point.z * point.z);
}

PlaneInfo plane_info(const LocalFramePlane plane)
{
    switch (plane)
    {
    case LocalFramePlane::XY:
        return {"XY", "x", "y", "z"};
    case LocalFramePlane::XZ:
        return {"XZ", "x", "z", "y"};
    case LocalFramePlane::YZ:
        return {"YZ", "y", "z", "x"};
    default:
        return {};
    }
}

const char* locked_axis_name(const LocalFramePlane plane)
{
    switch (plane)
    {
    case LocalFramePlane::XY:
        return "Z-axis";
    case LocalFramePlane::XZ:
        return "Y-axis";
    case LocalFramePlane::YZ:
        return "X-axis";
    default:
        return "axis";
    }
}

float first_component(const DirectX::XMFLOAT3& point, const LocalFramePlane plane)
{
    switch (plane)
    {
    case LocalFramePlane::XY:
    case LocalFramePlane::XZ:
        return point.x;
    case LocalFramePlane::YZ:
        return point.y;
    default:
        return 0.0f;
    }
}

float second_component(const DirectX::XMFLOAT3& point, const LocalFramePlane plane)
{
    switch (plane)
    {
    case LocalFramePlane::XY:
        return point.y;
    case LocalFramePlane::XZ:
    case LocalFramePlane::YZ:
        return point.z;
    default:
        return 0.0f;
    }
}

DirectX::XMFLOAT3 point_on_plane(
    const LocalFramePlane plane,
    const float first,
    const float second,
    const float locked)
{
    switch (plane)
    {
    case LocalFramePlane::XY:
        return {first, second, locked};
    case LocalFramePlane::XZ:
        return {first, locked, second};
    case LocalFramePlane::YZ:
        return {locked, first, second};
    default:
        return {};
    }
}

float locked_component(const DirectX::XMFLOAT3& point, const LocalFramePlane plane)
{
    switch (plane)
    {
    case LocalFramePlane::XY:
        return point.z;
    case LocalFramePlane::XZ:
        return point.y;
    case LocalFramePlane::YZ:
        return point.x;
    default:
        return 0.0f;
    }
}
} // namespace

LocalFrameSimulator::LocalFrameSimulator(std::shared_ptr<LocalFrameState> state)
    : state_(std::move(state))
{
    if (!state_)
    {
        throw std::invalid_argument("LocalFrameSimulator requires shared state.");
    }
}

const char* LocalFrameSimulator::name() const noexcept
{
    return "Local Frame Lab - Stage 2";
}

void LocalFrameSimulator::update(const SimulationTick& tick)
{
    elapsed_seconds_ = tick.total_seconds;
}

void LocalFrameSimulator::render_ui(const SimulatorFrameContext& context)
{
    handle_plane_drag();
    draw_scene_labels();

    ImGui::SetNextWindowPos(ImVec2(24.0f, 24.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(460.0f, 365.0f), ImGuiCond_FirstUseEver);

    ImGui::Begin(name());
    ImGui::TextUnformatted("Plane-slice interaction inside a 3D lattice.");
    ImGui::Separator();

    int plane_index = static_cast<int>(state_->active_plane);
    if (ImGui::RadioButton("XY", plane_index == static_cast<int>(LocalFramePlane::XY)))
    {
        set_active_plane(LocalFramePlane::XY);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("XZ", plane_index == static_cast<int>(LocalFramePlane::XZ)))
    {
        set_active_plane(LocalFramePlane::XZ);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("YZ", plane_index == static_cast<int>(LocalFramePlane::YZ)))
    {
        set_active_plane(LocalFramePlane::YZ);
    }

    const PlaneInfo info = plane_info(state_->active_plane);
    float active_components[2] = {
        first_component(state_->point, state_->active_plane),
        second_component(state_->point, state_->active_plane),
    };

    char slider_label[64] = {};
    std::snprintf(
        slider_label,
        sizeof(slider_label),
        "P.%s / P.%s",
        info.first_label,
        info.second_label);
    if (ImGui::SliderFloat2(slider_label, active_components, -k_grid_extent, k_grid_extent, "%.2f"))
    {
        set_point_from_active_components(active_components[0], active_components[1]);
    }

    if (ImGui::Button("Reset P"))
    {
        reset_point();
    }

    ImGui::SameLine();
    ImGui::Checkbox("Component legs", &state_->show_component_legs);
    ImGui::Checkbox("Depth hints", &state_->show_lattice_depth);

    ImGui::Separator();
    ImGui::Text("P = (%.2f, %.2f, %.2f)", state_->point.x, state_->point.y, state_->point.z);
    ImGui::Text("|P| = sqrt(x^2 + y^2 + z^2) = %.3f", magnitude(state_->point));
    ImGui::Text(
        "Distance along %s: |%s| = %.3f",
        info.first_label,
        info.first_label,
        std::abs(active_components[0]));
    ImGui::Text(
        "Distance along %s: |%s| = %.3f",
        info.second_label,
        info.second_label,
        std::abs(active_components[1]));
    ImGui::Text(
        "Active slice: %s, preserving %s = %.2f",
        info.name,
        info.locked_label,
        locked_component(state_->point, state_->active_plane));
    ImGui::Text("Client area: %u x %u", context.client_width, context.client_height);
    ImGui::Text("Elapsed: %.2f seconds", elapsed_seconds_);
    ImGui::Text("Click or drag the scene background to move P on the %s plane.", info.name);
    ImGui::End();
}

void LocalFrameSimulator::handle_plane_drag()
{
    const ImGuiIO& io = ImGui::GetIO();
    if (!state_->picking_ready)
    {
        return;
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        dragging_point_ = false;
    }

    if (!dragging_point_ && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !io.WantCaptureMouse)
    {
        dragging_point_ = true;
    }

    if (dragging_point_ && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        move_point_to_mouse();
    }
}

void LocalFrameSimulator::move_point_to_mouse()
{
    const ImGuiIO& io = ImGui::GetIO();

    if (state_->viewport_width == 0 || state_->viewport_height == 0)
    {
        return;
    }

    const float ndc_x =
        (2.0f * io.MousePos.x / static_cast<float>(state_->viewport_width)) - 1.0f;
    const float ndc_y =
        1.0f - (2.0f * io.MousePos.y / static_cast<float>(state_->viewport_height));

    using namespace DirectX;
    const XMMATRIX inverse_view_projection = XMLoadFloat4x4(&state_->inverse_view_projection);
    const XMVECTOR near_point =
        XMVector3TransformCoord(XMVectorSet(ndc_x, ndc_y, 0.0f, 1.0f), inverse_view_projection);
    const XMVECTOR far_point =
        XMVector3TransformCoord(XMVectorSet(ndc_x, ndc_y, 1.0f, 1.0f), inverse_view_projection);
    const XMVECTOR direction = XMVectorSubtract(far_point, near_point);

    XMFLOAT3 near_world = {};
    XMFLOAT3 ray_direction = {};
    XMStoreFloat3(&near_world, near_point);
    XMStoreFloat3(&ray_direction, direction);

    float near_plane_value = 0.0f;
    float direction_plane_value = 0.0f;
    switch (state_->active_plane)
    {
    case LocalFramePlane::XY:
        near_plane_value = near_world.z;
        direction_plane_value = ray_direction.z;
        break;
    case LocalFramePlane::XZ:
        near_plane_value = near_world.y;
        direction_plane_value = ray_direction.y;
        break;
    case LocalFramePlane::YZ:
        near_plane_value = near_world.x;
        direction_plane_value = ray_direction.x;
        break;
    }

    if (std::abs(direction_plane_value) < 0.0001f)
    {
        return;
    }

    const float locked = locked_component(state_->point, state_->active_plane);
    const float t = (locked - near_plane_value) / direction_plane_value;
    const float x = near_world.x + (ray_direction.x * t);
    const float y = near_world.y + (ray_direction.y * t);
    const float z = near_world.z + (ray_direction.z * t);

    switch (state_->active_plane)
    {
    case LocalFramePlane::XY:
        state_->point = {
            std::clamp(x, -k_grid_extent, k_grid_extent),
            std::clamp(y, -k_grid_extent, k_grid_extent),
            locked};
        break;
    case LocalFramePlane::XZ:
        state_->point = {
            std::clamp(x, -k_grid_extent, k_grid_extent),
            locked,
            std::clamp(z, -k_grid_extent, k_grid_extent)};
        break;
    case LocalFramePlane::YZ:
        state_->point = {
            locked,
            std::clamp(y, -k_grid_extent, k_grid_extent),
            std::clamp(z, -k_grid_extent, k_grid_extent)};
        break;
    }
}

void LocalFrameSimulator::draw_scene_labels() const
{
    if (!state_->picking_ready || state_->viewport_width == 0 || state_->viewport_height == 0)
    {
        return;
    }

    draw_active_plane_overlay();

    auto* draw_list = ImGui::GetBackgroundDrawList();
    const auto add_text = [draw_list](const ImVec2 position, const ImU32 color, const char* text)
    {
        const ImU32 shadow = IM_COL32(0, 0, 0, 210);
        draw_list->AddText(ImVec2(position.x + 1.0f, position.y + 1.0f), shadow, text);
        draw_list->AddText(position, color, text);
    };

    ImVec2 point_screen = {};
    if (world_to_screen(state_->point, point_screen))
    {
        char label[96] = {};
        std::snprintf(
            label,
            sizeof(label),
            "P  x = %.2f, y = %.2f, z = %.2f",
            state_->point.x,
            state_->point.y,
            state_->point.z);
        add_text(ImVec2(point_screen.x + 10.0f, point_screen.y - 20.0f), IM_COL32(255, 245, 190, 255), label);
    }

    if (state_->show_component_legs)
    {
        const PlaneInfo info = plane_info(state_->active_plane);
        const float first = first_component(state_->point, state_->active_plane);
        const float second = second_component(state_->point, state_->active_plane);
        const float locked = locked_component(state_->point, state_->active_plane);

        ImVec2 second_measure_screen = {};
        const DirectX::XMFLOAT3 second_measure_midpoint = point_on_plane(
            state_->active_plane,
            first,
            second * 0.5f,
            locked);
        if (world_to_screen(second_measure_midpoint, second_measure_screen))
        {
            char label[48] = {};
            std::snprintf(label, sizeof(label), "|%s| = %.2f", info.second_label, std::abs(second));
            add_text(ImVec2(second_measure_screen.x + 8.0f, second_measure_screen.y - 8.0f), IM_COL32(135, 245, 155, 255), label);
        }

        ImVec2 first_measure_screen = {};
        const DirectX::XMFLOAT3 first_measure_midpoint = point_on_plane(
            state_->active_plane,
            first * 0.5f,
            second,
            locked);
        if (world_to_screen(first_measure_midpoint, first_measure_screen))
        {
            char label[48] = {};
            std::snprintf(label, sizeof(label), "|%s| = %.2f", info.first_label, std::abs(first));
            add_text(ImVec2(first_measure_screen.x + 8.0f, first_measure_screen.y - 8.0f), IM_COL32(255, 150, 105, 255), label);
        }
    }

    const PlaneInfo info = plane_info(state_->active_plane);
    const float locked = locked_component(state_->point, state_->active_plane);
    const DirectX::XMFLOAT3 slice_anchor =
        point_on_plane(state_->active_plane, 0.0f, 0.0f, locked);
    ImVec2 slice_anchor_screen = {};
    if (world_to_screen(slice_anchor, slice_anchor_screen))
    {
        char label[80] = {};
        std::snprintf(
            label,
            sizeof(label),
            "%s intersects %s",
            locked_axis_name(state_->active_plane),
            info.name);
        add_text(
            ImVec2(slice_anchor_screen.x + 10.0f, slice_anchor_screen.y + 10.0f),
            IM_COL32(215, 190, 255, 255),
            label);
    }

    struct AxisLabel
    {
        DirectX::XMFLOAT3 world = {};
        const char* text = "";
        ImU32 color = 0;
    };

    const AxisLabel axis_labels[] = {
        {{6.7f, 0.0f, 0.0f}, "X", IM_COL32(245, 70, 70, 255)},
        {{0.0f, 6.7f, 0.0f}, "Y", IM_COL32(80, 235, 110, 255)},
        {{0.0f, 0.0f, 4.7f}, "Z", IM_COL32(100, 150, 255, 255)},
    };

    for (const AxisLabel& axis : axis_labels)
    {
        ImVec2 screen = {};
        if (world_to_screen(axis.world, screen))
        {
            add_text(ImVec2(screen.x + 4.0f, screen.y - 4.0f), axis.color, axis.text);
        }
    }
}

void LocalFrameSimulator::draw_active_plane_overlay() const
{
    const float locked = locked_component(state_->point, state_->active_plane);
    const DirectX::XMFLOAT3 corners_world[] = {
        point_on_plane(state_->active_plane, -k_grid_extent, -k_grid_extent, locked),
        point_on_plane(state_->active_plane, k_grid_extent, -k_grid_extent, locked),
        point_on_plane(state_->active_plane, k_grid_extent, k_grid_extent, locked),
        point_on_plane(state_->active_plane, -k_grid_extent, k_grid_extent, locked),
    };

    ImVec2 corners_screen[4] = {};
    for (int i = 0; i < 4; ++i)
    {
        if (!world_to_screen(corners_world[i], corners_screen[i]))
        {
            return;
        }
    }

    ImGui::GetBackgroundDrawList()->AddConvexPolyFilled(
        corners_screen,
        4,
        IM_COL32(95, 135, 180, 22));
}

void LocalFrameSimulator::set_active_plane(const LocalFramePlane plane)
{
    if (state_->active_plane == plane)
    {
        return;
    }

    state_->active_plane = plane;
    dragging_point_ = false;
}

void LocalFrameSimulator::set_point_from_active_components(const float first, const float second)
{
    state_->point = point_on_plane(
        state_->active_plane,
        std::clamp(first, -k_grid_extent, k_grid_extent),
        std::clamp(second, -k_grid_extent, k_grid_extent),
        locked_component(state_->point, state_->active_plane));
}

bool LocalFrameSimulator::world_to_screen(
    const DirectX::XMFLOAT3 world,
    ImVec2& screen) const
{
    using namespace DirectX;

    const XMMATRIX view_projection = XMLoadFloat4x4(&state_->view_projection);
    const XMVECTOR projected = XMVector3TransformCoord(
        XMVectorSet(world.x, world.y, world.z, 1.0f),
        view_projection);

    XMFLOAT3 ndc = {};
    XMStoreFloat3(&ndc, projected);

    if (ndc.x < -1.2f || ndc.x > 1.2f || ndc.y < -1.2f || ndc.y > 1.2f)
    {
        return false;
    }

    screen.x = (ndc.x * 0.5f + 0.5f) * static_cast<float>(state_->viewport_width);
    screen.y = (0.5f - ndc.y * 0.5f) * static_cast<float>(state_->viewport_height);
    return true;
}

void LocalFrameSimulator::reset_point()
{
    state_->point = {3.0f, 2.0f, 0.0f};
}
} // namespace physics::sim
