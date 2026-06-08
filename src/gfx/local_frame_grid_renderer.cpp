#include "gfx/local_frame_grid_renderer.h"

#include <d3dcompiler.h>

#include <algorithm>
#include <cstring>
#include <stdexcept>

#include "third_party/imgui/imgui.h"

using Microsoft::WRL::ComPtr;

namespace physics::gfx
{
namespace
{
constexpr unsigned int k_max_vertices = 2048;
constexpr float k_grid_extent = 6.0f;
constexpr float k_depth_extent = 4.0f;

constexpr char k_shader_source[] = R"(
cbuffer SceneConstants : register(b0)
{
    row_major float4x4 view_projection;
};

struct VSInput
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.position = mul(float4(input.position, 1.0), view_projection);
    output.color = input.color;
    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}
)";

void throw_if_failed(const HRESULT hr, const char* message)
{
    if (FAILED(hr))
    {
        throw std::runtime_error(message);
    }
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
        k_shader_source,
        std::strlen(k_shader_source),
        nullptr,
        nullptr,
        nullptr,
        entry_point,
        target,
        flags,
        0,
        &bytecode,
        &errors);

    if (FAILED(hr))
    {
        const char* message = errors ? static_cast<const char*>(errors->GetBufferPointer())
                                     : "D3DCompile failed.";
        throw std::runtime_error(message);
    }

    return bytecode;
}

unsigned int align_constant_buffer_size(const unsigned int size)
{
    return (size + 255u) & ~255u;
}

D3D12_HEAP_PROPERTIES upload_heap_properties()
{
    D3D12_HEAP_PROPERTIES properties = {};
    properties.Type = D3D12_HEAP_TYPE_UPLOAD;
    properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    properties.CreationNodeMask = 1;
    properties.VisibleNodeMask = 1;
    return properties;
}

D3D12_RESOURCE_DESC buffer_desc(const unsigned long long byte_count)
{
    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Alignment = 0;
    desc.Width = byte_count;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    desc.Flags = D3D12_RESOURCE_FLAG_NONE;
    return desc;
}
} // namespace

LocalFrameGridRenderer::LocalFrameGridRenderer(std::shared_ptr<sim::LocalFrameState> state)
    : state_(std::move(state))
{
    if (!state_)
    {
        throw std::invalid_argument("LocalFrameGridRenderer requires shared state.");
    }
}

LocalFrameGridRenderer::~LocalFrameGridRenderer()
{
    if (vertex_buffer_)
    {
        vertex_buffer_->Unmap(0, nullptr);
    }

    if (constant_buffer_)
    {
        constant_buffer_->Unmap(0, nullptr);
    }
}

const char* LocalFrameGridRenderer::name() const noexcept
{
    return "Stage 1: 3D Grid Lines";
}

void LocalFrameGridRenderer::initialize(const RendererInitContext& context)
{
    width_ = context.initial_width;
    height_ = context.initial_height;
    build_pipeline(context);
    create_upload_buffers(context.device);
}

void LocalFrameGridRenderer::on_resize(const RendererResizeContext& context)
{
    width_ = context.width;
    height_ = context.height;
}

void LocalFrameGridRenderer::record_draw(const RendererFrameContext& context)
{
    if (!pipeline_state_ || !root_signature_ || !mapped_vertices_ || !mapped_constants_)
    {
        return;
    }

    width_ = context.width;
    height_ = context.height;
    update_camera(context);
    build_vertices();

    if (vertices_.empty())
    {
        return;
    }

    const std::size_t vertex_count = std::min<std::size_t>(vertices_.size(), k_max_vertices);
    std::memcpy(mapped_vertices_, vertices_.data(), vertex_count * sizeof(Vertex));

    context.command_list->SetGraphicsRootSignature(root_signature_.Get());
    context.command_list->SetPipelineState(pipeline_state_.Get());

    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(context.width);
    viewport.Height = static_cast<float>(context.height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    D3D12_RECT scissor = {};
    scissor.left = 0;
    scissor.top = 0;
    scissor.right = static_cast<LONG>(context.width);
    scissor.bottom = static_cast<LONG>(context.height);

    context.command_list->RSSetViewports(1, &viewport);
    context.command_list->RSSetScissorRects(1, &scissor);
    context.command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    context.command_list->IASetVertexBuffers(0, 1, &vertex_buffer_view_);
    context.command_list->SetGraphicsRootConstantBufferView(
        0,
        constant_buffer_->GetGPUVirtualAddress());
    context.command_list->DrawInstanced(static_cast<UINT>(vertex_count), 1, 0, 0);
}

void LocalFrameGridRenderer::render_ui()
{
    ImGui::TextUnformatted("Line renderer: lattice, axes, P, and component legs.");
    ImGui::Text("Viewport: %u x %u", width_, height_);
    ImGui::Text("Vertices: %zu", vertices_.size());
}

void LocalFrameGridRenderer::build_pipeline(const RendererInitContext& context)
{
    D3D12_ROOT_PARAMETER root_parameter = {};
    root_parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    root_parameter.Descriptor.ShaderRegister = 0;
    root_parameter.Descriptor.RegisterSpace = 0;
    root_parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    D3D12_ROOT_SIGNATURE_DESC root_signature_desc = {};
    root_signature_desc.NumParameters = 1;
    root_signature_desc.pParameters = &root_parameter;
    root_signature_desc.NumStaticSamplers = 0;
    root_signature_desc.pStaticSamplers = nullptr;
    root_signature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> errors;
    throw_if_failed(
        D3D12SerializeRootSignature(
            &root_signature_desc,
            D3D_ROOT_SIGNATURE_VERSION_1,
            &signature,
            &errors),
        errors ? static_cast<const char*>(errors->GetBufferPointer())
               : "D3D12SerializeRootSignature failed.");

    throw_if_failed(
        context.device->CreateRootSignature(
            0,
            signature->GetBufferPointer(),
            signature->GetBufferSize(),
            IID_PPV_ARGS(&root_signature_)),
        "CreateRootSignature failed.");

    const ComPtr<ID3DBlob> vertex_shader = compile_shader("VSMain", "vs_5_0");
    const ComPtr<ID3DBlob> pixel_shader = compile_shader("PSMain", "ps_5_0");

    D3D12_INPUT_ELEMENT_DESC input_elements[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
    pso_desc.pRootSignature = root_signature_.Get();
    pso_desc.VS = {vertex_shader->GetBufferPointer(), vertex_shader->GetBufferSize()};
    pso_desc.PS = {pixel_shader->GetBufferPointer(), pixel_shader->GetBufferSize()};
    pso_desc.BlendState.AlphaToCoverageEnable = FALSE;
    pso_desc.BlendState.IndependentBlendEnable = FALSE;
    pso_desc.BlendState.RenderTarget[0].BlendEnable = FALSE;
    pso_desc.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
    pso_desc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
    pso_desc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
    pso_desc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    pso_desc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    pso_desc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    pso_desc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    pso_desc.BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
    pso_desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    pso_desc.SampleMask = UINT_MAX;
    pso_desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    pso_desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    pso_desc.RasterizerState.FrontCounterClockwise = FALSE;
    pso_desc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    pso_desc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    pso_desc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    pso_desc.RasterizerState.DepthClipEnable = TRUE;
    pso_desc.RasterizerState.MultisampleEnable = FALSE;
    pso_desc.RasterizerState.AntialiasedLineEnable = TRUE;
    pso_desc.RasterizerState.ForcedSampleCount = 0;
    pso_desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    pso_desc.DepthStencilState.DepthEnable = FALSE;
    pso_desc.DepthStencilState.StencilEnable = FALSE;
    pso_desc.InputLayout = {input_elements, 2};
    pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    pso_desc.NumRenderTargets = 1;
    pso_desc.RTVFormats[0] = context.back_buffer_format;
    pso_desc.SampleDesc.Count = 1;

    throw_if_failed(
        context.device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&pipeline_state_)),
        "CreateGraphicsPipelineState failed.");
}

void LocalFrameGridRenderer::create_upload_buffers(ID3D12Device* device)
{
    const D3D12_HEAP_PROPERTIES heap_properties = upload_heap_properties();
    const D3D12_RESOURCE_DESC vertex_desc = buffer_desc(k_max_vertices * sizeof(Vertex));
    throw_if_failed(
        device->CreateCommittedResource(
            &heap_properties,
            D3D12_HEAP_FLAG_NONE,
            &vertex_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&vertex_buffer_)),
        "CreateCommittedResource vertex buffer failed.");

    D3D12_RANGE no_read = {};
    throw_if_failed(vertex_buffer_->Map(0, &no_read, reinterpret_cast<void**>(&mapped_vertices_)),
                    "Vertex buffer Map failed.");

    vertex_buffer_view_.BufferLocation = vertex_buffer_->GetGPUVirtualAddress();
    vertex_buffer_view_.SizeInBytes = k_max_vertices * sizeof(Vertex);
    vertex_buffer_view_.StrideInBytes = sizeof(Vertex);

    const unsigned int constant_buffer_size =
        align_constant_buffer_size(static_cast<unsigned int>(sizeof(SceneConstants)));
    const D3D12_RESOURCE_DESC constant_desc = buffer_desc(constant_buffer_size);
    throw_if_failed(
        device->CreateCommittedResource(
            &heap_properties,
            D3D12_HEAP_FLAG_NONE,
            &constant_desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&constant_buffer_)),
        "CreateCommittedResource constant buffer failed.");

    throw_if_failed(
        constant_buffer_->Map(0, &no_read, reinterpret_cast<void**>(&mapped_constants_)),
        "Constant buffer Map failed.");
}

void LocalFrameGridRenderer::update_camera(const RendererFrameContext& context)
{
    using namespace DirectX;

    const float aspect = context.height == 0
        ? 1.0f
        : static_cast<float>(context.width) / static_cast<float>(context.height);
    const XMVECTOR eye = XMVectorSet(7.5f, -9.0f, 6.5f, 1.0f);
    const XMVECTOR target = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    const XMVECTOR up = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    const XMMATRIX view = XMMatrixLookAtLH(eye, target, up);
    const XMMATRIX projection = XMMatrixOrthographicLH(13.5f * aspect, 13.5f, 0.1f, 100.0f);
    const XMMATRIX view_projection = XMMatrixMultiply(view, projection);
    const XMMATRIX inverse_view_projection = XMMatrixInverse(nullptr, view_projection);

    XMStoreFloat4x4(&mapped_constants_->view_projection, view_projection);
    XMStoreFloat4x4(&state_->view_projection, view_projection);
    XMStoreFloat4x4(&state_->inverse_view_projection, inverse_view_projection);
    state_->viewport_width = context.width;
    state_->viewport_height = context.height;
    state_->picking_ready = true;
}

void LocalFrameGridRenderer::build_vertices()
{
    vertices_.clear();
    vertices_.reserve(192);

    const DirectX::XMFLOAT4 grid_color = {0.20f, 0.23f, 0.27f, 1.0f};
    const DirectX::XMFLOAT4 depth_color = {0.13f, 0.16f, 0.20f, 1.0f};
    const DirectX::XMFLOAT4 x_color = {0.95f, 0.18f, 0.18f, 1.0f};
    const DirectX::XMFLOAT4 y_color = {0.22f, 0.86f, 0.35f, 1.0f};
    const DirectX::XMFLOAT4 z_color = {0.25f, 0.48f, 1.0f, 1.0f};
    const DirectX::XMFLOAT4 vector_color = {1.0f, 0.86f, 0.24f, 1.0f};
    const DirectX::XMFLOAT4 point_color = {1.0f, 1.0f, 1.0f, 1.0f};
    const DirectX::XMFLOAT4 x_measure_color = {0.98f, 0.48f, 0.32f, 1.0f};
    const DirectX::XMFLOAT4 y_measure_color = {0.52f, 0.95f, 0.58f, 1.0f};

    for (int i = -6; i <= 6; ++i)
    {
        const float p = static_cast<float>(i);
        add_line({-k_grid_extent, p, 0.0f}, {k_grid_extent, p, 0.0f}, grid_color);
        add_line({p, -k_grid_extent, 0.0f}, {p, k_grid_extent, 0.0f}, grid_color);
    }

    if (state_->show_lattice_depth)
    {
        for (int i = -6; i <= 6; i += 3)
        {
            const float p = static_cast<float>(i);
            add_line({p, k_grid_extent, -k_depth_extent}, {p, k_grid_extent, k_depth_extent}, depth_color);
            add_line({-k_grid_extent, p, -k_depth_extent}, {-k_grid_extent, p, k_depth_extent}, depth_color);
        }

        add_line({-k_grid_extent, k_grid_extent, k_depth_extent}, {k_grid_extent, k_grid_extent, k_depth_extent}, depth_color);
        add_line({-k_grid_extent, -k_grid_extent, k_depth_extent}, {-k_grid_extent, k_grid_extent, k_depth_extent}, depth_color);
        add_line({-k_grid_extent, k_grid_extent, -k_depth_extent}, {k_grid_extent, k_grid_extent, -k_depth_extent}, depth_color);
        add_line({-k_grid_extent, -k_grid_extent, -k_depth_extent}, {-k_grid_extent, k_grid_extent, -k_depth_extent}, depth_color);
    }

    add_line({-k_grid_extent - 0.5f, 0.0f, 0.0f}, {k_grid_extent + 0.5f, 0.0f, 0.0f}, x_color);
    add_line({0.0f, -k_grid_extent - 0.5f, 0.0f}, {0.0f, k_grid_extent + 0.5f, 0.0f}, y_color);
    add_line({0.0f, 0.0f, -k_depth_extent - 0.5f}, {0.0f, 0.0f, k_depth_extent + 0.5f}, z_color);

    const DirectX::XMFLOAT3 point = state_->point;
    if (state_->show_component_legs)
    {
        add_line({0.0f, 0.0f, 0.0f}, {point.x, 0.0f, 0.0f}, x_color);
        add_line({point.x, 0.0f, 0.0f}, {point.x, point.y, 0.0f}, y_measure_color);
        add_line({0.0f, point.y, 0.0f}, {point.x, point.y, 0.0f}, x_measure_color);
    }

    add_line({0.0f, 0.0f, 0.0f}, point, vector_color);

    constexpr float marker = 0.18f;
    add_line({point.x - marker, point.y, point.z}, {point.x + marker, point.y, point.z}, point_color);
    add_line({point.x, point.y - marker, point.z}, {point.x, point.y + marker, point.z}, point_color);
    add_line({point.x, point.y, point.z - marker}, {point.x, point.y, point.z + marker}, point_color);
}

void LocalFrameGridRenderer::add_line(
    DirectX::XMFLOAT3 start,
    DirectX::XMFLOAT3 end,
    DirectX::XMFLOAT4 color)
{
    if (vertices_.size() + 2 > k_max_vertices)
    {
        return;
    }

    vertices_.push_back(Vertex{start, color});
    vertices_.push_back(Vertex{end, color});
}
} // namespace physics::gfx
