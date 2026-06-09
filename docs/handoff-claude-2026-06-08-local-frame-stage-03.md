# Claude Handoff: Local Frame Lab Stage 3

Date: 2026-06-08

## Start Here

Read these first:

1. `CLAUDE.md`
2. `docs/agent-instructions.md`
3. `docs/local-frame-lab-stage-catalog.md`
4. `docs/lesson-06-local-frame-stage-03-free-3d-vector.md`

## Current Architecture

Same as Stage 2:

- `ISimulator`: model state, update behavior, simulator-owned ImGui
- `ISimulatorRenderer`: D3D12 resources, PSO/root signature/shaders/draw calls, renderer-owned ImGui
- `SimulatorModule`: one simulator paired with one renderer

## Active Files

- `src/sim/local_frame_simulator.cpp`
- `src/gfx/local_frame_grid_renderer.cpp`
- `src/sim/local_frame_state.h`
- `src/app/default_simulator_module.cpp`

## Stage 1 / Stage 2 Status

Preserved. Toggle "Free 3D mode" off in the ImGui panel to access Stage 2 plane-slice behavior.

## Stage 3 Status

Implemented:

- `free_3d_mode` flag (default true) in `LocalFrameState`
- Orbit camera: `camera_yaw`, `camera_pitch`, `camera_distance` in state; perspective projection in renderer
- Drag background → orbit; scroll → zoom
- 3 independent x/y/z sliders for P
- 3-axis staircase component legs (red X, green Y, blue Z)
- 12-edge box visualization (`show_box`)
- Face diagonals through origin corner (`show_face_diagonals`)
- Axis-angle / direction-cosine readouts (alpha, beta, gamma, cos²+cos²+cos²)
- Lesson: `docs/lesson-06-local-frame-stage-03-free-3d-vector.md`

## Verification

Last successful build:

```powershell
cmake --build build --config Debug --target physics_sandbox
```

## Next Stage

Stage 4: Rotated Local 3D Frame

Expected additions:

- local basis vectors i', j', k'
- yaw/pitch/roll or equivalent rotation controls
- local coordinate readouts (x', y', z')
- component projections onto the rotated basis
- show that P stays fixed while local-frame components change
- optional animated basis rotation
- keep Stage 1, 2, 3 lessons intact
- create a Stage 4 lesson before or during implementation

Do not skip Stage 3 preservation before starting Stage 4.

## User Preferences

- Build in named stages; preserve each with lesson docs.
- Keep lessons why-first, chaptered, visual, formula-aware, small.
- No renderer selector unless user asks.
- C++23, RAII, smart pointers, CPU-first, explicit ownership.
- Visual payoff matters.

## Gotchas

- ImGui mouse guard: check `io.WantCaptureMouse` before scene interaction.
- `state_->picking_ready` must be true before any ray-picking or labeling.
- Orbit drag and scroll both check `!io.WantCaptureMouse`.
- Perspective projection is active in free_3d_mode; orthographic in plane mode. The shared `inverse_view_projection` is used for plane-mode picking only; free-mode editing uses sliders.
- `SECRET_SCAN_REPORT.md` — remind owner to review secret-management concerns.
