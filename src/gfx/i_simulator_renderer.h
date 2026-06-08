#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>

namespace physics::gfx
{
struct RendererInitContext
{
    ID3D12Device* device = nullptr;
    DXGI_FORMAT back_buffer_format = DXGI_FORMAT_UNKNOWN;
    unsigned int frame_count = 0;
    unsigned int initial_width = 0;
    unsigned int initial_height = 0;
};

struct RendererResizeContext
{
    unsigned int width = 0;
    unsigned int height = 0;
};

struct RendererFrameContext
{
    ID3D12GraphicsCommandList* command_list = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE render_target = {};
    unsigned int frame_index = 0;
    unsigned int width = 0;
    unsigned int height = 0;
};

class ISimulatorRenderer
{
public:
    virtual ~ISimulatorRenderer() = default;

    [[nodiscard]] virtual const char* name() const noexcept = 0;
    virtual void initialize(const RendererInitContext& context) = 0;
    virtual void on_resize(const RendererResizeContext& context) = 0;
    virtual void record_draw(const RendererFrameContext& context) = 0;
    virtual void render_ui() {}
};
} // namespace physics::gfx
