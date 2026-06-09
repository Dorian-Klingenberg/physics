#include "gfx/line_renderer_base.h"

#include <d3dcompiler.h>

#include <algorithm>
#include <cstring>
#include <stdexcept>

using Microsoft::WRL::ComPtr;

namespace physics::gfx
{
namespace
{

// Vertex-color pass-through shader used by every line renderer.
constexpr char k_shader_source[] = R"(
cbuffer SceneConstants : register(b0)
{
    row_major float4x4 view_projection;
};

struct VSInput { float3 position : POSITION; float4 color : COLOR; };
struct PSInput { float4 position : SV_POSITION; float4 color : COLOR; };

PSInput VSMain(VSInput v)
{
    PSInput o;
    o.position = mul(float4(v.position, 1.0), view_projection);
    o.color    = v.color;
    return o;
}

float4 PSMain(PSInput i) : SV_TARGET { return i.color; }
)";

void throw_if_failed(const HRESULT hr, const char* message)
{
    if (FAILED(hr))
        throw std::runtime_error(message);
}

ComPtr<ID3DBlob> compile_shader(const char* entry_point, const char* target)
{
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    ComPtr<ID3DBlob> bytecode;
    ComPtr<ID3DBlob> errors;
    const HRESULT hr = D3DCompile(
        k_shader_source, std::strlen(k_shader_source),
        nullptr, nullptr, nullptr,
        entry_point, target, flags, 0,
        &bytecode, &errors);
    if (FAILED(hr))
    {
        const char* msg = errors
            ? static_cast<const char*>(errors->GetBufferPointer())
            : "D3DCompile failed.";
        throw std::runtime_error(msg);
    }
    return bytecode;
}

unsigned int align_cb(const unsigned int size)
{
    return (size + 255u) & ~255u;
}

D3D12_HEAP_PROPERTIES upload_heap()
{
    D3D12_HEAP_PROPERTIES p = {};
    p.Type                  = D3D12_HEAP_TYPE_UPLOAD;
    p.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    p.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
    p.CreationNodeMask      = 1;
    p.VisibleNodeMask       = 1;
    return p;
}

D3D12_RESOURCE_DESC buffer_desc(const unsigned long long bytes)
{
    D3D12_RESOURCE_DESC d = {};
    d.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    d.Width              = bytes;
    d.Height             = 1;
    d.DepthOrArraySize   = 1;
    d.MipLevels          = 1;
    d.Format             = DXGI_FORMAT_UNKNOWN;
    d.SampleDesc.Count   = 1;
    d.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    return d;
}

} // namespace

LineRendererBase::LineRendererBase(const unsigned int max_vertices)
    : max_vertices_(max_vertices)
{
}

LineRendererBase::~LineRendererBase()
{
    if (vertex_buffer_)
        vertex_buffer_->Unmap(0, nullptr);
    if (constant_buffer_)
        constant_buffer_->Unmap(0, nullptr);
}

void LineRendererBase::initialize(const RendererInitContext& context)
{
    width_  = context.initial_width;
    height_ = context.initial_height;
    build_pipeline(context);
    create_upload_buffers(context.device);
}

void LineRendererBase::on_resize(const RendererResizeContext& context)
{
    width_  = context.width;
    height_ = context.height;
}

void LineRendererBase::record_draw(const RendererFrameContext& context)
{
    if (!pipeline_state_ || !root_signature_ || !mapped_vertices_ || !mapped_constants_)
        return;

    width_  = context.width;
    height_ = context.height;

    update_camera(context);
    build_vertices();

    if (vertices_.empty())
        return;

    const std::size_t vertex_count =
        std::min<std::size_t>(vertices_.size(), max_vertices_);
    std::memcpy(mapped_vertices_, vertices_.data(), vertex_count * sizeof(Vertex));

    context.command_list->SetGraphicsRootSignature(root_signature_.Get());
    context.command_list->SetPipelineState(pipeline_state_.Get());

    D3D12_VIEWPORT vp = {};
    vp.Width    = static_cast<float>(context.width);
    vp.Height   = static_cast<float>(context.height);
    vp.MaxDepth = 1.0f;

    D3D12_RECT scissor = {};
    scissor.right  = static_cast<LONG>(context.width);
    scissor.bottom = static_cast<LONG>(context.height);

    context.command_list->RSSetViewports(1, &vp);
    context.command_list->RSSetScissorRects(1, &scissor);
    context.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    context.command_list->IASetVertexBuffers(0, 1, &vertex_buffer_view_);
    context.command_list->SetGraphicsRootConstantBufferView(
        0, constant_buffer_->GetGPUVirtualAddress());
    context.command_list->DrawInstanced(static_cast<UINT>(vertex_count), 1, 0, 0);
}

void LineRendererBase::add_line(
    const DirectX::XMFLOAT3 start,
    const DirectX::XMFLOAT3 end,
    const DirectX::XMFLOAT4 color)
{
    if (vertices_.size() + 2 > max_vertices_)
        return;
    vertices_.push_back(Vertex{start, color});
    vertices_.push_back(Vertex{end,   color});
}

void LineRendererBase::build_pipeline(const RendererInitContext& context)
{
    D3D12_ROOT_PARAMETER root_param = {};
    root_param.ParameterType              = D3D12_ROOT_PARAMETER_TYPE_CBV;
    root_param.Descriptor.ShaderRegister  = 0;
    root_param.ShaderVisibility           = D3D12_SHADER_VISIBILITY_VERTEX;

    D3D12_ROOT_SIGNATURE_DESC rs_desc = {};
    rs_desc.NumParameters = 1;
    rs_desc.pParameters   = &root_param;
    rs_desc.Flags         = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> sig;
    ComPtr<ID3DBlob> errors;
    throw_if_failed(
        D3D12SerializeRootSignature(&rs_desc, D3D_ROOT_SIGNATURE_VERSION_1, &sig, &errors),
        errors ? static_cast<const char*>(errors->GetBufferPointer())
               : "D3D12SerializeRootSignature failed.");
    throw_if_failed(
        context.device->CreateRootSignature(
            0, sig->GetBufferPointer(), sig->GetBufferSize(),
            IID_PPV_ARGS(&root_signature_)),
        "CreateRootSignature failed.");

    const ComPtr<ID3DBlob> vs = compile_shader("VSMain", "vs_5_0");
    const ComPtr<ID3DBlob> ps = compile_shader("PSMain", "ps_5_0");

    D3D12_INPUT_ELEMENT_DESC inputs[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso = {};
    pso.pRootSignature    = root_signature_.Get();
    pso.VS                = {vs->GetBufferPointer(), vs->GetBufferSize()};
    pso.PS                = {ps->GetBufferPointer(), ps->GetBufferSize()};
    pso.SampleMask        = UINT_MAX;

    pso.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    pso.BlendState.RenderTarget[0].SrcBlend  = D3D12_BLEND_ONE;
    pso.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
    pso.BlendState.RenderTarget[0].BlendOp   = D3D12_BLEND_OP_ADD;
    pso.BlendState.RenderTarget[0].SrcBlendAlpha  = D3D12_BLEND_ONE;
    pso.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    pso.BlendState.RenderTarget[0].BlendOpAlpha   = D3D12_BLEND_OP_ADD;

    pso.RasterizerState.FillMode              = D3D12_FILL_MODE_SOLID;
    pso.RasterizerState.CullMode              = D3D12_CULL_MODE_NONE;
    pso.RasterizerState.DepthClipEnable       = TRUE;
    pso.RasterizerState.AntialiasedLineEnable = TRUE;

    pso.DepthStencilState.DepthEnable   = FALSE;
    pso.DepthStencilState.StencilEnable = FALSE;

    pso.InputLayout           = {inputs, 2};
    pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    pso.NumRenderTargets      = 1;
    pso.RTVFormats[0]         = context.back_buffer_format;
    pso.SampleDesc.Count      = 1;

    throw_if_failed(
        context.device->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&pipeline_state_)),
        "CreateGraphicsPipelineState failed.");
}

void LineRendererBase::create_upload_buffers(ID3D12Device* device)
{
    const D3D12_HEAP_PROPERTIES heap = upload_heap();
    D3D12_RANGE no_read              = {};

    const D3D12_RESOURCE_DESC vb_desc = buffer_desc(max_vertices_ * sizeof(Vertex));
    throw_if_failed(
        device->CreateCommittedResource(
            &heap, D3D12_HEAP_FLAG_NONE, &vb_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&vertex_buffer_)),
        "CreateCommittedResource vertex buffer failed.");
    throw_if_failed(
        vertex_buffer_->Map(0, &no_read, reinterpret_cast<void**>(&mapped_vertices_)),
        "Vertex buffer Map failed.");

    vertex_buffer_view_.BufferLocation = vertex_buffer_->GetGPUVirtualAddress();
    vertex_buffer_view_.SizeInBytes    = max_vertices_ * sizeof(Vertex);
    vertex_buffer_view_.StrideInBytes  = sizeof(Vertex);

    const unsigned int cb_size = align_cb(static_cast<unsigned int>(sizeof(SceneConstants)));
    const D3D12_RESOURCE_DESC cb_desc = buffer_desc(cb_size);
    throw_if_failed(
        device->CreateCommittedResource(
            &heap, D3D12_HEAP_FLAG_NONE, &cb_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&constant_buffer_)),
        "CreateCommittedResource constant buffer failed.");
    throw_if_failed(
        constant_buffer_->Map(0, &no_read, reinterpret_cast<void**>(&mapped_constants_)),
        "Constant buffer Map failed.");
}

} // namespace physics::gfx
