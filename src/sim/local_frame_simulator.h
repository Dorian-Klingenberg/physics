#pragma once

#include <memory>

#include "sim/i_simulator.h"
#include "sim/local_frame_state.h"

struct ImVec2;

namespace physics::sim
{
class LocalFrameSimulator final : public ISimulator
{
public:
    explicit LocalFrameSimulator(std::shared_ptr<LocalFrameState> state);

    [[nodiscard]] const char* name() const noexcept override;
    void update(const SimulationTick& tick) override;
    void render_ui(const SimulatorFrameContext& context) override;

private:
    void render_free_3d_ui(const SimulatorFrameContext& context);
    void render_plane_slice_ui();
    void handle_interaction();
    void move_point_to_mouse();
    void move_point_on_drag_plane();
    [[nodiscard]] DirectX::XMFLOAT3 compute_camera_look_dir() const;
    void draw_scene_labels() const;
    void draw_active_plane_overlay() const;
    void set_active_plane(LocalFramePlane plane);
    void set_point_from_active_components(float first, float second);
    [[nodiscard]] bool world_to_screen(
        DirectX::XMFLOAT3 world,
        ImVec2& screen) const;
    void reset_point();

    std::shared_ptr<LocalFrameState> state_;
    float elapsed_seconds_ = 0.0f;
    bool dragging_point_ = false;
    bool orbiting_ = false;
    DirectX::XMFLOAT3 drag_plane_normal_ = {};
    DirectX::XMFLOAT3 drag_plane_origin_ = {};
};
} // namespace physics::sim
