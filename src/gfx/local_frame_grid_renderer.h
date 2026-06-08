#pragma once

#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl/client.h>

#include <memory>
#include <vector>

#include "gfx/i_simulator_renderer.h"
#include "sim/local_frame_state.h"

namespace physics::gfx
{
class LocalFrameGridRenderer final : public ISimulatorRenderer
{
public:
    explicit LocalFrameGridRenderer(std::shared_ptr<sim::LocalFrameState> state);
    ~LocalFrameGridRenderer() override;

    [[nodiscard]] const char* name() const noexcept override;
    void initialize(const RendererInitContext& context) override;
    void on_resize(const RendererResizeContext& context) override;
    void record_draw(const RendererFrameContext& context) override;
    void render_ui() override;

private:
    struct Vertex
    {
        DirectX::XMFLOAT3 position = {};
        DirectX::XMFLOAT4 color = {};
    };

    struct SceneConstants
    {
        DirectX::XMFLOAT4X4 view_projection = {};
    };

    void build_pipeline(const RendererInitContext& context);
    void create_upload_buffers(ID3D12Device* device);
    void update_camera(const RendererFrameContext& context);
    void build_vertices();
    void add_line(
        DirectX::XMFLOAT3 start,
        DirectX::XMFLOAT3 end,
        DirectX::XMFLOAT4 color);

    std::shared_ptr<sim::LocalFrameState> state_;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline_state_;
    Microsoft::WRL::ComPtr<ID3D12Resource> vertex_buffer_;
    Microsoft::WRL::ComPtr<ID3D12Resource> constant_buffer_;
    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_ = {};
    Vertex* mapped_vertices_ = nullptr;
    SceneConstants* mapped_constants_ = nullptr;
    std::vector<Vertex> vertices_;
    unsigned int width_ = 0;
    unsigned int height_ = 0;
};
} // namespace physics::gfx
