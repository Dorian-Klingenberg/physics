#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>

#include "sim/empty_simulator.h"
#include "sim/i_simulator.h"
#include "third_party/imgui/imgui.h"
#include "third_party/imgui/backends/imgui_impl_dx12.h"
#include "third_party/imgui/backends/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

using Microsoft::WRL::ComPtr;

namespace
{
constexpr wchar_t k_window_class_name[] = L"PhysicsSandboxWindow";
constexpr wchar_t k_window_title[] = L"Physics Sandbox - DirectX 12 + ImGui";
constexpr UINT k_frame_count = 2;
constexpr DXGI_FORMAT k_back_buffer_format = DXGI_FORMAT_R8G8B8A8_UNORM;

void throw_if_failed(const HRESULT hr, const char* message)
{
    if (FAILED(hr))
    {
        throw std::runtime_error(message);
    }
}

struct FrameContext
{
    ComPtr<ID3D12CommandAllocator> allocator;
    UINT64 fence_value = 0;
};

struct Application;
Application* g_app = nullptr;

LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

std::unique_ptr<physics::sim::ISimulator> create_simulator()
{
    // Swap this implementation when you want to plug in a different simulator.
    return std::make_unique<physics::sim::EmptySimulator>();
}

struct Application
{
    HWND hwnd = nullptr;
    UINT width = 1280;
    UINT height = 720;
    bool minimized = false;
    bool d3d_ready = false;

    ComPtr<IDXGIFactory4> factory;
    ComPtr<ID3D12Device> device;
    ComPtr<ID3D12CommandQueue> command_queue;
    ComPtr<IDXGISwapChain3> swap_chain;

    ComPtr<ID3D12DescriptorHeap> rtv_heap;
    ComPtr<ID3D12DescriptorHeap> imgui_srv_heap;
    UINT rtv_descriptor_size = 0;

    std::array<ComPtr<ID3D12Resource>, k_frame_count> render_targets;
    std::array<FrameContext, k_frame_count> frames;
    ComPtr<ID3D12GraphicsCommandList> command_list;

    ComPtr<ID3D12Fence> fence;
    HANDLE fence_event = nullptr;
    UINT64 next_fence_value = 1;
    UINT frame_index = 0;
    std::unique_ptr<physics::sim::ISimulator> simulator;

    Application()
        : simulator(create_simulator())
    {
        g_app = this;
    }

    ~Application()
    {
        if (device)
        {
            wait_for_gpu();
        }

        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        if (fence_event)
        {
            CloseHandle(fence_event);
            fence_event = nullptr;
        }

        g_app = nullptr;
    }

    void create_window()
    {
        WNDCLASSEXW window_class = {};
        window_class.cbSize = sizeof(window_class);
        window_class.style = CS_HREDRAW | CS_VREDRAW;
        window_class.lpfnWndProc = window_proc;
        window_class.hInstance = GetModuleHandleW(nullptr);
        window_class.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
        window_class.lpszClassName = k_window_class_name;

        if (!RegisterClassExW(&window_class))
        {
            throw std::runtime_error("RegisterClassExW failed.");
        }

        RECT window_rect = {0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
        AdjustWindowRectEx(&window_rect, WS_OVERLAPPEDWINDOW, FALSE, 0);

        hwnd = CreateWindowExW(
            0,
            k_window_class_name,
            k_window_title,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            window_rect.right - window_rect.left,
            window_rect.bottom - window_rect.top,
            nullptr,
            nullptr,
            GetModuleHandleW(nullptr),
            nullptr);

        if (!hwnd)
        {
            throw std::runtime_error("CreateWindowExW failed.");
        }

        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
    }

    void initialize_d3d12()
    {
        UINT factory_flags = 0;
#if defined(_DEBUG)
        {
            ComPtr<ID3D12Debug> debug_controller;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller))))
            {
                debug_controller->EnableDebugLayer();
                factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
            }
        }
#endif

        throw_if_failed(
            CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&factory)),
            "CreateDXGIFactory2 failed.");

        throw_if_failed(
            D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)),
            "D3D12CreateDevice failed.");

        D3D12_COMMAND_QUEUE_DESC queue_desc = {};
        queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queue_desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        throw_if_failed(
            device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue)),
            "CreateCommandQueue failed.");

        DXGI_SWAP_CHAIN_DESC1 swap_desc = {};
        swap_desc.BufferCount = k_frame_count;
        swap_desc.Width = width;
        swap_desc.Height = height;
        swap_desc.Format = k_back_buffer_format;
        swap_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swap_desc.SampleDesc.Count = 1;

        ComPtr<IDXGISwapChain1> swap_chain1;
        throw_if_failed(
            factory->CreateSwapChainForHwnd(
                command_queue.Get(), hwnd, &swap_desc, nullptr, nullptr, &swap_chain1),
            "CreateSwapChainForHwnd failed.");
        throw_if_failed(swap_chain1.As(&swap_chain), "IDXGISwapChain3 query failed.");

        factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

        frame_index = swap_chain->GetCurrentBackBufferIndex();

        D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
        rtv_heap_desc.NumDescriptors = k_frame_count;
        rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        throw_if_failed(
            device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&rtv_heap)),
            "CreateDescriptorHeap (RTV) failed.");
        rtv_descriptor_size =
            device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        create_render_target_views();

        for (FrameContext& frame : frames)
        {
            throw_if_failed(
                device->CreateCommandAllocator(
                    D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frame.allocator)),
                "CreateCommandAllocator failed.");
        }

        throw_if_failed(
            device->CreateCommandList(
                0,
                D3D12_COMMAND_LIST_TYPE_DIRECT,
                frames[0].allocator.Get(),
                nullptr,
                IID_PPV_ARGS(&command_list)),
            "CreateCommandList failed.");
        throw_if_failed(command_list->Close(), "Initial command list Close failed.");

        throw_if_failed(
            device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)),
            "CreateFence failed.");

        fence_event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
        if (!fence_event)
        {
            throw std::runtime_error("CreateEventW failed.");
        }

        d3d_ready = true;
    }

    void initialize_imgui()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        if (!ImGui_ImplWin32_Init(hwnd))
        {
            throw std::runtime_error("ImGui_ImplWin32_Init failed.");
        }

        D3D12_DESCRIPTOR_HEAP_DESC srv_heap_desc = {};
        srv_heap_desc.NumDescriptors = 1;
        srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        throw_if_failed(
            device->CreateDescriptorHeap(&srv_heap_desc, IID_PPV_ARGS(&imgui_srv_heap)),
            "CreateDescriptorHeap (ImGui SRV) failed.");

        if (!ImGui_ImplDX12_Init(
                device.Get(),
                k_frame_count,
                k_back_buffer_format,
                imgui_srv_heap.Get(),
                imgui_srv_heap->GetCPUDescriptorHandleForHeapStart(),
                imgui_srv_heap->GetGPUDescriptorHandleForHeapStart()))
        {
            throw std::runtime_error("ImGui_ImplDX12_Init failed.");
        }
    }

    void create_render_target_views()
    {
        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle =
            rtv_heap->GetCPUDescriptorHandleForHeapStart();

        for (UINT i = 0; i < k_frame_count; ++i)
        {
            throw_if_failed(
                swap_chain->GetBuffer(i, IID_PPV_ARGS(&render_targets[i])),
                "Swap-chain GetBuffer failed.");
            device->CreateRenderTargetView(render_targets[i].Get(), nullptr, rtv_handle);
            rtv_handle.ptr += rtv_descriptor_size;
        }
    }

    void release_render_targets()
    {
        for (ComPtr<ID3D12Resource>& target : render_targets)
        {
            target.Reset();
        }
    }

    void wait_for_gpu()
    {
        if (!command_queue || !fence || !fence_event)
        {
            return;
        }

        const UINT64 fence_to_wait_for = next_fence_value++;
        throw_if_failed(
            command_queue->Signal(fence.Get(), fence_to_wait_for),
            "CommandQueue Signal failed.");

        if (fence->GetCompletedValue() < fence_to_wait_for)
        {
            throw_if_failed(
                fence->SetEventOnCompletion(fence_to_wait_for, fence_event),
                "Fence SetEventOnCompletion failed.");
            WaitForSingleObjectEx(fence_event, INFINITE, FALSE);
        }
    }

    void resize(UINT new_width, UINT new_height)
    {
        width = new_width;
        height = new_height;
        minimized = (new_width == 0 || new_height == 0);

        if (minimized || !d3d_ready)
        {
            return;
        }

        wait_for_gpu();
        release_render_targets();

        for (FrameContext& frame : frames)
        {
            frame.fence_value = 0;
        }

        throw_if_failed(
            swap_chain->ResizeBuffers(
                k_frame_count, width, height, k_back_buffer_format, 0),
            "Swap-chain ResizeBuffers failed.");

        frame_index = swap_chain->GetCurrentBackBufferIndex();
        create_render_target_views();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE current_rtv_handle() const
    {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = rtv_heap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += static_cast<SIZE_T>(frame_index) * rtv_descriptor_size;
        return handle;
    }

    void begin_frame()
    {
        FrameContext& frame = frames[frame_index];

        if (frame.fence_value != 0 && fence->GetCompletedValue() < frame.fence_value)
        {
            throw_if_failed(
                fence->SetEventOnCompletion(frame.fence_value, fence_event),
                "Fence wait before allocator reset failed.");
            WaitForSingleObjectEx(fence_event, INFINITE, FALSE);
        }

        throw_if_failed(frame.allocator->Reset(), "Command allocator Reset failed.");
        throw_if_failed(
            command_list->Reset(frame.allocator.Get(), nullptr),
            "Command list Reset failed.");

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = render_targets[frame_index].Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        command_list->ResourceBarrier(1, &barrier);

        const D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = current_rtv_handle();
        command_list->OMSetRenderTargets(1, &rtv_handle, FALSE, nullptr);
    }

    void end_frame()
    {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = render_targets[frame_index].Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        command_list->ResourceBarrier(1, &barrier);

        throw_if_failed(command_list->Close(), "Command list Close failed.");

        ID3D12CommandList* lists[] = {command_list.Get()};
        command_queue->ExecuteCommandLists(1, lists);

        throw_if_failed(swap_chain->Present(1, 0), "Swap-chain Present failed.");

        frames[frame_index].fence_value = next_fence_value;
        throw_if_failed(
            command_queue->Signal(fence.Get(), next_fence_value),
            "CommandQueue Signal after Present failed.");
        ++next_fence_value;

        frame_index = swap_chain->GetCurrentBackBufferIndex();
    }

    void draw_simulator_ui()
    {
        const ImGuiIO& io = ImGui::GetIO();
        const physics::sim::SimulatorFrameContext context{
            width,
            height,
            frame_index,
            io.DisplaySize.x,
            io.DisplaySize.y,
        };
        simulator->render_ui(context);
    }

    void render()
    {
        constexpr float clear_color[4] = {0.06f, 0.07f, 0.09f, 1.0f};

        begin_frame();

        const D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = current_rtv_handle();
        command_list->ClearRenderTargetView(rtv_handle, clear_color, 0, nullptr);

        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        draw_simulator_ui();
        ImGui::Render();

        ID3D12DescriptorHeap* heaps[] = {imgui_srv_heap.Get()};
        command_list->SetDescriptorHeaps(1, heaps);
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), command_list.Get());

        end_frame();
    }

    void run()
    {
        MSG msg = {};
        bool running = true;

        while (running)
        {
            while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                if (msg.message == WM_QUIT)
                {
                    running = false;
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }

            if (!running)
            {
                break;
            }

            if (!minimized)
            {
                render();
            }
            else
            {
                WaitMessage();
            }
        }
    }
};

LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wparam, lparam))
    {
        return 1;
    }

    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_SIZE:
        if (g_app && wparam != SIZE_MINIMIZED)
        {
            const UINT new_width = LOWORD(lparam);
            const UINT new_height = HIWORD(lparam);
            g_app->resize(new_width, new_height);
        }
        else if (g_app)
        {
            g_app->minimized = true;
        }
        return 0;

    default:
        return DefWindowProcW(hwnd, message, wparam, lparam);
    }
}
} // namespace

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    try
    {
        Application app;
        app.create_window();
        app.initialize_d3d12();
        app.initialize_imgui();
        app.run();
        return 0;
    }
    catch (const std::exception& error)
    {
        MessageBoxA(nullptr, error.what(), "Physics Sandbox Error", MB_OK | MB_ICONERROR);
        return 1;
    }
}
