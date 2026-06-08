# CLAUDE.md

Claude should follow the same repository instructions as Codex and Copilot:

- `docs/agent-instructions.md`

Short version:

- This is a C++23 / DirectX 12 / Dear ImGui physics sandbox.
- The project builds playful, visual, honest toy models, not scientific papers.
- Use `ISimulator` for model/update/simulator UI.
- Use `ISimulatorRenderer` for D3D12 renderer resources and draw calls.
- Use `SimulatorModule` to pair one simulator with one renderer.
- Preserve simulator work in named stages with lesson docs.
- Build the Local Frame Lab from `docs/local-frame-lab-stage-catalog.md`: XY plane in 3D, plane slices, free 3D vector, rotated local 3D frame.
