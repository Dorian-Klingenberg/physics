#include "sim/empty_simulator.h"

#include "third_party/imgui/imgui.h"

namespace physics::sim
{
const char* EmptySimulator::name() const noexcept
{
    return "Empty Simulator";
}

void EmptySimulator::render_ui(const SimulatorFrameContext& context)
{
    ImGui::SetNextWindowPos(ImVec2(24.0f, 24.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(430.0f, 180.0f), ImGuiCond_FirstUseEver);

    ImGui::Begin(name());
    ImGui::TextUnformatted("Step 01: resizable Win32 + D3D12 + ImGui.");
    ImGui::TextUnformatted("Step 02: simulator-owned ImGui panel.");
    ImGui::Separator();
    ImGui::Text("Client area: %u x %u", context.client_width, context.client_height);
    ImGui::Text("Frame index: %u", context.frame_index);
    ImGui::Text(
        "ImGui DisplaySize: %.0f x %.0f",
        context.imgui_display_width,
        context.imgui_display_height);
    ImGui::TextUnformatted("Resize the window: this panel keeps its own size.");
    ImGui::End();
}
} // namespace physics::sim
