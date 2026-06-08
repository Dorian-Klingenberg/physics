#include "gfx/empty_renderer.h"

#include "third_party/imgui/imgui.h"

namespace physics::gfx
{
const char* EmptyRenderer::name() const noexcept
{
    return "Empty Renderer";
}

void EmptyRenderer::initialize(const RendererInitContext& context)
{
    width_ = context.initial_width;
    height_ = context.initial_height;
}

void EmptyRenderer::on_resize(const RendererResizeContext& context)
{
    width_ = context.width;
    height_ = context.height;
}

void EmptyRenderer::record_draw(const RendererFrameContext& context)
{
    (void)context;
}

void EmptyRenderer::render_ui()
{
    ImGui::TextUnformatted("Renderer owns no GPU resources yet.");
    ImGui::Text("Renderer size: %u x %u", width_, height_);
}
} // namespace physics::gfx
