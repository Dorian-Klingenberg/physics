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
    void handle_xy_plane_click();
    void move_point_to_mouse();
    void draw_scene_labels() const;
    [[nodiscard]] bool world_to_screen(
        DirectX::XMFLOAT3 world,
        ImVec2& screen) const;
    void reset_point();

    std::shared_ptr<LocalFrameState> state_;
    float elapsed_seconds_ = 0.0f;
    bool dragging_point_ = false;
};
} // namespace physics::sim
