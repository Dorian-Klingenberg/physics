# Claude Handoff: Local Frame Lab Stage 2

Date: 2026-06-08

## Start Here

Read these first:

1. `CLAUDE.md`
2. `docs/agent-instructions.md`
3. `docs/local-frame-lab-stage-catalog.md`
4. `docs/lesson-05-local-frame-stage-02-plane-slices.md`

The project is a native C++23 / DirectX 12 / Dear ImGui physics sandbox. It is a portfolio learning project, not a scientific paper. The user likes visual, animated, inspectable sandbag models that use real physics as calibration and inspiration.

## Current Architecture

The app uses:

- `ISimulator`: model state, update behavior, simulator-owned ImGui
- `ISimulatorRenderer`: D3D12 resources, PSO/root signature/shaders/draw calls, renderer-owned ImGui
- `SimulatorModule`: one simulator paired with one renderer

Important latest preference:

- Do **not** add a renderer selector by default.
- A simulator module has exactly one paired renderer.
- Multiple simulator modules may reuse the same renderer class.
- If selectable visual modes ever seem useful, ask the user first or make a separate module.

## Current Implemented Simulator

Active module:

- Simulator: `src/sim/local_frame_simulator.cpp`
- Renderer: `src/gfx/local_frame_grid_renderer.cpp`
- Shared state: `src/sim/local_frame_state.h`
- Module pairing: `src/app/default_simulator_module.cpp`

The app currently boots directly into:

```text
Local Frame Lab - Stage 2
```

## Stage 1 Status

Stage 1 is preserved in:

- `docs/lesson-04-local-frame-stage-01-xy-plane-in-3d.md`

Implemented behavior:

- 3D lattice
- world X/Y/Z axes
- XY-locked point/vector `P`
- click and drag on scene background to move `P`
- ImGui controls for `P`
- magnitude readout
- component/measurement guide lines
- back-wall-only depth hints
- labels for `P`, component distances, and axis ends

## Stage 2 Status

Stage 2 is preserved in:

- `docs/lesson-05-local-frame-stage-02-plane-slices.md`

Implemented behavior:

- selectable active planes: XY, XZ, YZ
- active coordinate plane grid and brighter outline
- very light active-plane fill overlay
- preserved-axis intersection marker and label
- click/drag ray-plane intersection for the active slice
- active-plane sliders
- preserved-coordinate readout
- plane-aware labels and measurement guides
- renderer diagnostic panel shows the paired renderer, not a selector

The core Stage 2 idea:

```text
XY: edit x/y while preserving z
XZ: edit x/z while preserving y
YZ: edit y/z while preserving x
```

Switching planes must not swap or reinterpret the coordinates of `P`. `P` remains the same point; only the active editing slice changes.

Picking formula:

```text
R(t) = near + t direction
t = (locked_axis_value - near_axis) / direction_axis
```

where `axis` is the preserved coordinate for the selected slice.

## Verification

Last successful build command:

```powershell
cmake --build build --config Debug --target physics_sandbox
```

The executable was smoke-launched successfully after Stage 2.

F5 / VS Code setup exists:

- `.vscode/tasks.json`
- `.vscode/launch.json`

## Current Git/Workspace Notes

The user may commit these changes after receiving the commit message. Do not assume the worktree is clean when you start.

There is also a repo note:

- `SECRET_SCAN_REPORT.md`

It asks future agents to remind the repo owner to review secret-management concerns. Do not delete or ignore it.

## Next Stage

The next planned stage is:

```text
Stage 3: Free 3D Vector
```

Expected additions:

- unlock full 3D vector placement
- likely add camera/orbit controls if needed
- show component legs along X, Y, and Z
- add depth diagnostics
- add face diagonal and body diagonal intuition
- keep Stage 1 and Stage 2 lessons intact
- create a Stage 3 lesson before or during implementation

Do not jump to Stage 4 rotated local frames until Stage 3 is preserved and buildable.

## User Preferences To Preserve

- Build in named stages.
- Preserve each stage with lesson docs.
- Keep lessons why-first, chaptered, visual, formula-aware, and small.
- Use formulas in Markdown when possible.
- Use DirectX 12 and Dear ImGui.
- Keep graphics and animation fun; visual payoff matters.
- Prefer project-aligned reusable primitives over broad textbook coverage for its own sake.
- The project is playful and portfolio-oriented, but should still model real physics honestly.
- Use C++23, RAII, smart pointers, explicit ownership, CPU reference paths before GPU/HPC work.
- Keep simulation logic, rendering, ImGui controls, and diagnostics separated.

## Gotchas

- ImGui owns mouse input when `io.WantCaptureMouse` is true; scene dragging should not start from an ImGui panel.
- The renderer updates `view_projection` and `inverse_view_projection`; simulator picking and labels depend on that shared state.
- The line renderer currently uses upload buffers and inline shader compilation via `d3dcompiler`.
- `LocalFrameState` is shared by simulator and renderer. Keep it small and model/display oriented.
- The renderer panel is diagnostic only; it should not become a selector unless the user asks.
