# Agent Instructions

## Project Shape

This repository is a portfolio-grade C++23 physics sandbox. It builds playful, visual, inspectable toy models inspired by real physics. The goal is not to write scientific papers or prove new physics. The goal is to learn complex physics ideas by implementing honest sandbag models with strong engineering practice.

Prefer:

- C++23
- DirectX 12
- Dear ImGui
- RAII and explicit ownership
- smart pointers
- CPU reference paths before GPU optimization
- GPU/HPC work when the data shape justifies it

Do not default to web apps, notebooks, Unity, Unreal, or generic visualization stacks unless the user explicitly asks.

## Architecture

Use the Granny Sandbox 3 pattern adapted for this broader physics sandbox:

- `ISimulator` owns model state, update behavior, and simulator-specific ImGui controls.
- `ISimulatorRenderer` owns D3D12 resources, pipeline state, shaders, draw commands, and renderer-specific ImGui controls.
- `SimulatorModule` pairs one simulator with one renderer.

Do not add a renderer selector to a simulator module. If multiple simulators need the same rendering style, reuse the same renderer class in multiple modules. If a simulator genuinely needs a different visual mode, create a different module or decide explicitly with the user before adding renderer selection.

Do not assume every renderer should work with every simulator. A 2D/3D vector-grid lab, scalar-field wave lab, particle orbit lab, and quantum-inspired state lab may each need different renderer classes.

The app shell should stay boring and stable:

- Win32 window lifetime
- D3D12 device, command queue, swap chain, fences, and back-buffer transitions
- ImGui backend setup
- active simulator module

## Stage Preservation

Build simulators in named stages and preserve each stage before moving on.

For every simulator stage:

- create or update a lesson document for the stage,
- keep the completed stage buildable before adding the next stage,
- avoid overwriting the lesson narrative for earlier stages,
- preserve important code snapshots when a later stage will substantially change the implementation,
- document what the stage teaches, what controls exist, what visual output changed, and what reusable primitive was created.

When an arc grows beyond a couple of lessons, add a catalog file listing the stages in order. Stage preservation matters because this project is a learning portfolio, not just an end-state app.

## Local Frame Lab Plan

The first real simulator should be the Local Frame Lab. The canonical stage catalog is `docs/local-frame-lab-stage-catalog.md`. Build it in four preserved stages:

1. **Stage 1: XY Plane In A 3D World**
   - 3D lattice visible, but interaction constrained to the XY plane.
   - World X/Y/Z axes visible.
   - One selected point/vector.
   - Readouts for world coordinates and magnitude.

2. **Stage 2: Plane Slices**
   - Add XY, XZ, and YZ plane-lock modes.
   - Keep the interaction simple while making the third dimension real.
   - Show projection shadows on the active plane.

3. **Stage 3: Free 3D Vector**
   - Unlock full 3D point/vector placement.
   - Add camera/orbit controls if needed.
   - Show depth guides, component legs, face diagonals, and body diagonal intuition.

4. **Stage 4: Rotated Local 3D Frame**
   - Add local basis vectors `i'`, `j'`, and `k'`.
   - Add yaw/pitch/roll or equivalent rotation controls.
   - Show that the vector stays fixed while its local-frame components change.
   - Animate basis rotation when useful.

Default to making the scene visually interesting, but keep each stage small enough to teach one main idea.

## Lesson Style

Follow `docs/lesson-authoring-guide.md`.

Lessons should be:

- why-first
- chaptered
- formula-aware, with formulas in Markdown where possible
- visual, with diagrams when they clarify runtime/data flow
- explicit about what real physics says, what the toy model does, and where it breaks
- paired with code that is small, inspectable, and buildable

## Simulator Selection Taste

Follow `docs/simulation-selection-preferences.md`.

Pick textbook questions that become reusable simulation primitives, especially:

- frames and coordinate transforms
- vectors, projections, and basis changes
- particles, fields, waves, clocks, gradients, and probability-like states
- diagnostics that compare real expectations to toy behavior
- ideas that can grow from simple CPU models into GPU/HPC implementations

Graphics and animation are part of the motivation. Prefer simulations that move, rotate, accumulate, interfere, converge, or fail visibly.

## Future Engineering Knowledge Base

The planned local engineering RAG system is documented in
`docs/local-engineering-rag-plan.md`. It is not implemented yet.

When it is eventually built, use it as a local standards and best-practices
retrieval layer for C++23, RAII, C++ Core Guidelines, Win32/DirectX 12, clean
architecture, and design-pattern guidance. Treat project house rules and hard
C++/platform sources as stronger than generic clean-code advice.

Do not apply design patterns merely because they were retrieved. Use them only
when they clarify a real variation point, boundary, construction policy, or
ownership problem.
