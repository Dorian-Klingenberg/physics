# AGENTS.md

All Codex and other coding agents should follow the shared project instructions in:

- `docs/agent-instructions.md`

Key reminders:

- Build native C++23 / DirectX 12 / Dear ImGui simulators unless the user asks otherwise.
- Use the `ISimulator` + `ISimulatorRenderer` + `SimulatorModule` architecture.
- Pair one simulator with one renderer per module; do not add a renderer selector by default.
- Build simulator arcs in named, preserved stages.
- For the first Local Frame Lab, use `docs/local-frame-lab-stage-catalog.md`.
- Follow the lesson style in `docs/lesson-authoring-guide.md`.
- Follow simulator selection taste in `docs/simulation-selection-preferences.md`.
