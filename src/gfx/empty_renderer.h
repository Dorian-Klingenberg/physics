#pragma once

#include "gfx/i_simulator_renderer.h"

namespace physics::gfx
{
class EmptyRenderer final : public ISimulatorRenderer
{
public:
    [[nodiscard]] const char* name() const noexcept override;
    void initialize(const RendererInitContext& context) override;
    void on_resize(const RendererResizeContext& context) override;
    void record_draw(const RendererFrameContext& context) override;
    void render_ui() override;

private:
    unsigned int width_ = 0;
    unsigned int height_ = 0;
};
} // namespace physics::gfx
