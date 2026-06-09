#pragma once

#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl/client.h>

#include <vector>

#include "gfx/i_simulator_renderer.h"

namespace physics::gfx
{

// Owns all D3D12 line-rendering machinery: shader, PSO, upload buffers, draw call.
// Subclasses implement update_camera() and build_vertices() only.
//
// Usage:
//   override update_camera() — write view_projection into mapped_constants_
//   override build_vertices() — call add_line() to fill vertices_
//   width_ and height_ hold the current viewport size

class LineRendererBase : public ISimulatorRenderer
{
public:
    explicit LineRendererBase(unsigned int max_vertices = 4096);
    ~LineRendererBase() override;

    void initialize(const RendererInitContext& context) override;
    void on_resize(const RendererResizeContext& context) override;
    void record_draw(const RendererFrameContext& context) final;

protected:
    struct Vertex
    {
        DirectX::XMFLOAT3 position = {};
        DirectX::XMFLOAT4 color    = {};
    };

    struct SceneConstants
    {
        DirectX::XMFLOAT4X4 view_projection = {};
    };

    virtual void update_camera(const RendererFrameContext& context) = 0;
    virtual void build_vertices() = 0;

    void add_line(DirectX::XMFLOAT3 start, DirectX::XMFLOAT3 end, DirectX::XMFLOAT4 color);

    std::vector<Vertex> vertices_;
    SceneConstants*     mapped_constants_ = nullptr;
    unsigned int        width_            = 0;
    unsigned int        height_           = 0;

private:
    void build_pipeline(const RendererInitContext& context);
    void create_upload_buffers(ID3D12Device* device);

    unsigned int max_vertices_;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> root_signature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipeline_state_;
    Microsoft::WRL::ComPtr<ID3D12Resource>      vertex_buffer_;
    Microsoft::WRL::ComPtr<ID3D12Resource>      constant_buffer_;
    D3D12_VERTEX_BUFFER_VIEW                    vertex_buffer_view_ = {};
    Vertex*                                     mapped_vertices_    = nullptr;
};

} // namespace physics::gfx
