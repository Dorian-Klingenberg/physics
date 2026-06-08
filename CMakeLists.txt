cmake_minimum_required(VERSION 3.25)

project(physics_sandbox LANGUAGES CXX)

add_executable(physics_sandbox WIN32
    src/main.cpp
    src/app/default_simulator_module.cpp
    src/gfx/empty_renderer.cpp
    src/gfx/local_frame_grid_renderer.cpp
    src/sim/empty_simulator.cpp
    src/sim/local_frame_simulator.cpp
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/backends/imgui_impl_dx12.cpp
    third_party/imgui/backends/imgui_impl_win32.cpp
)

target_compile_features(physics_sandbox PRIVATE cxx_std_23)

target_include_directories(physics_sandbox PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/backends
)

if(MSVC)
    target_compile_options(physics_sandbox PRIVATE /W4 /permissive-)
endif()

target_link_libraries(physics_sandbox PRIVATE
    d3d12
    d3dcompiler
    dxgi
    dwmapi
)

set_target_properties(physics_sandbox PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
