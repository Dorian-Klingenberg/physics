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

float magnitude(const DirectX::XMFLOAT3& point)
{
    return std::sqrt(
        point.x * point.x +
        point.y * point.y +
        point.z * point.z);
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
    return "Local Frame Lab - Stage 1";
}

void LocalFrameSimulator::update(const SimulationTick& tick)
{
    elapsed_seconds_ = tick.total_seconds;
}

void LocalFrameSimulator::render_ui(const SimulatorFrameContext& context)
{
    handle_xy_plane_click();
    draw_scene_labels();

    ImGui::SetNextWindowPos(ImVec2(24.0f, 24.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(440.0f, 310.0f), ImGuiCond_FirstUseEver);

    ImGui::Begin(name());
    ImGui::TextUnformatted("XY plane interaction inside a 3D lattice.");
    ImGui::Separator();

    float point[3] = {state_->point.x, state_->point.y, state_->point.z};
    if (ImGui::SliderFloat2("P.x / P.y", point, -k_grid_extent, k_grid_extent, "%.2f"))
    {
        state_->point.x = point[0];
        state_->point.y = point[1];
        state_->point.z = 0.0f;
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
    ImGui::Text("Distance from y-axis to P: |x| = %.3f", std::abs(state_->point.x));
    ImGui::Text("Distance from x-axis to P: |y| = %.3f", std::abs(state_->point.y));
    ImGui::Text("Active plane: XY, so z = %.1f", state_->point.z);
    ImGui::Text("Client area: %u x %u", context.client_width, context.client_height);
    ImGui::Text("Elapsed: %.2f seconds", elapsed_seconds_);
    ImGui::TextUnformatted("Click or drag the scene background to move P on the XY plane.");
    ImGui::End();
}

void LocalFrameSimulator::handle_xy_plane_click()
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

    if (std::abs(ray_direction.z) < 0.0001f)
    {
        return;
    }

    const float t = -near_world.z / ray_direction.z;
    const float x = near_world.x + (ray_direction.x * t);
    const float y = near_world.y + (ray_direction.y * t);

    state_->point.x = std::clamp(x, -k_grid_extent, k_grid_extent);
    state_->point.y = std::clamp(y, -k_grid_extent, k_grid_extent);
    state_->point.z = 0.0f;
}

void LocalFrameSimulator::draw_scene_labels() const
{
    if (!state_->picking_ready || state_->viewport_width == 0 || state_->viewport_height == 0)
    {
        return;
    }

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
            "P  x = %.2f, y = %.2f",
            state_->point.x,
            state_->point.y);
        add_text(ImVec2(point_screen.x + 10.0f, point_screen.y - 20.0f), IM_COL32(255, 245, 190, 255), label);
    }

    if (state_->show_component_legs)
    {
        ImVec2 x_measure_screen = {};
        const DirectX::XMFLOAT3 x_measure_midpoint = {
            state_->point.x,
            state_->point.y * 0.5f,
            0.0f,
        };
        if (world_to_screen(x_measure_midpoint, x_measure_screen))
        {
            char label[48] = {};
            std::snprintf(label, sizeof(label), "|y| = %.2f", std::abs(state_->point.y));
            add_text(ImVec2(x_measure_screen.x + 8.0f, x_measure_screen.y - 8.0f), IM_COL32(135, 245, 155, 255), label);
        }

        ImVec2 y_measure_screen = {};
        const DirectX::XMFLOAT3 y_measure_midpoint = {
            state_->point.x * 0.5f,
            state_->point.y,
            0.0f,
        };
        if (world_to_screen(y_measure_midpoint, y_measure_screen))
        {
            char label[48] = {};
            std::snprintf(label, sizeof(label), "|x| = %.2f", std::abs(state_->point.x));
            add_text(ImVec2(y_measure_screen.x + 8.0f, y_measure_screen.y - 8.0f), IM_COL32(255, 150, 105, 255), label);
        }
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
