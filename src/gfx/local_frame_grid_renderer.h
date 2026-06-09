#pragma once

#include <memory>

#include "gfx/line_renderer_base.h"
#include "sim/local_frame_state.h"

namespace physics::gfx
{

class LocalFrameGridRenderer final : public LineRendererBase
{
public:
    explicit LocalFrameGridRenderer(std::shared_ptr<sim::LocalFrameState> state);

    [[nodiscard]] const char* name() const noexcept override;
    void render_ui() override;

protected:
    void update_camera(const RendererFrameContext& context) override;
    void build_vertices() override;

private:
    void build_free_3d_vertices();
    void build_plane_slice_vertices();

    std::shared_ptr<sim::LocalFrameState> state_;
};

} // namespace physics::gfx
