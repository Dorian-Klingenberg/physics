# Copilot Instructions

Follow the shared repository instructions in:

- `docs/agent-instructions.md`

Important defaults:

- Prefer C++23, DirectX 12, Dear ImGui, RAII, smart pointers, and explicit ownership.
- Keep simulation logic, ImGui controls, rendering, and diagnostics separated.
- Use the `ISimulator` + `ISimulatorRenderer` + `SimulatorModule` architecture.
- Pair one simulator with one renderer per module.
- Do not add a renderer selector by default; reuse renderer classes across modules instead.
- Do not assume all renderers are globally compatible with all simulators.
- Preserve each simulator stage with lesson documentation before moving on.
- For the Local Frame Lab, follow `docs/local-frame-lab-stage-catalog.md`.
