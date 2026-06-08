# Simulation Selection Preferences

## Purpose

This note tells future agents how to choose textbook questions and turn them into small simulations for this project. The goal is not to solve every exercise. The goal is to pick questions that become reusable pieces of the larger physics sandbox.

The project is a portfolio-grade C++23 physics-inspired simulation sandbox. It uses real physics as calibration and inspiration, but it is not trying to prove new physics. Simulations should be playful, inspectable, and honest about where the toy model stops matching reality.

## What I Like

I prefer textbook questions that can become small, reusable simulation primitives.

Good picks usually do at least one of these:

- Build a core simulation concept: position, velocity, acceleration, force, field, frame, path, state update, energy, wave, probability, or metric.
- Connect directly to the larger project: time fields, gradient steering, particle motion, proper time, coordinate frames, field propagation, quantum-inspired states, materials, or HPC/GPU-friendly data.
- Produce something visual, animated, and interactive, not just a numeric answer.
- Teach a reusable mental model for later physics.
- Expose a useful diagnostic: what should happen, what the toy model does, and where the two diverge.
- Can be extended from a simple 2D version into a richer 3D, field, GPU, or relativistic version later.

Graphics and animation are part of the motivation. Prefer questions that can turn into something visually alive: moving particles, rotating frames, trails, vector arrows, fields, waves, color maps, clocks, changing parameters, or animated comparisons.

## What I Do Not Want

Avoid picking questions just because they are difficult or algebra-heavy.

Lower-priority picks are usually:

- one-off plug-and-chug calculations
- exercises that only produce a static number
- problems that teach a narrow trick but do not become a reusable simulator
- questions whose best implementation would be a worksheet, not an interactive model
- formal proof-style questions unless the proof reveals a visual or computational pattern

It is fine to fold these into a broader simulator as modes or test cases. They usually should not become standalone simulations.

## Selection Heuristics

When reviewing textbook questions, rank them by these criteria:

1. **Reusable primitive value**: Does this become a tool the sandbox will need again?
2. **Project alignment**: Does it support fields, frames, particles, proper time, waves, quantum behavior, materials, or optimization?
3. **Visual and animation payoff**: Can the user see the concept move, rotate, accumulate, interfere, converge, or fail?
4. **Diagnostic value**: Can the simulation compare a toy rule against a real physics expectation?
5. **Extension path**: Can it grow from a simple version into a serious portfolio module?
6. **Chapter coverage**: Does it help cover an important part of the chapter without becoming detached from the project?
7. **Implementation economy**: Can it be built cleanly without a large detour?

Prefer questions that score well on several of these, even if they are not the hardest problems in the chapter.

Broad chapter coverage is valuable, but it is not the highest priority. If a choice comes down to covering one more isolated textbook topic versus building a more visual, animated, reusable, project-aligned simulator, choose the project-aligned simulator. Coverage should serve the sandbox, not pull it off course.

## How To Convert a Question Into a Simulation

For each selected question, define:

```text
Concept:
What physics or math idea does this teach?

Sandbox role:
What reusable simulation primitive does it create?

Interactive controls:
What can the user drag, tune, pause, randomize, or compare?

Live outputs:
What vectors, fields, paths, charts, values, or errors are displayed?

Diagnostic:
What should happen according to real physics or math?

Break/extension:
Where can this model be pushed next?
```

The result should feel like a tiny instrument, not a homework answer.

## Chapter 3 Example Preferences

For the Chapter 3 vector problems, the preferred picks are not merely the most advanced vector calculations. The best picks are the ones that become scaffolding for later physics.

Highest-value examples:

- **Rotated coordinate systems** become local frames, observer frames, tensor components, and later relativity/metric intuition.
- **Position as a function of time** becomes the first motion kernel: position, velocity, derivative, and simulation stepping.
- **Iterative movement toward target points** becomes interpolation, relaxation, state updates, attractors, and numerical solver intuition.
- **Force-vector summation** becomes the force accumulator used later for gravity-like gradients, EM-like forces, drag, wind, and vortex terms.
- **Wind-relative motion** becomes frame-relative and medium-relative motion, useful for analogue-gravity thinking.
- **Path displacement** becomes trajectory vs. state-change intuition, useful for proper-time paths and field traversal.
- **3D diagonals/components** become 3D vector inspection and preparation for 3D fields, grids, and GPU layouts.
- **Equal-vector angle sweeps** become constructive/destructive combination intuition, useful later for wave interference.

Lower-value examples can still be used as validation cases inside these broader simulators.

## Preferred Simulator Style

Each small simulator should aim for:

- a minimal but real physics core
- clear separation between simulation state and visualization
- deterministic stepping where possible
- visible vectors, paths, frames, animation, and numeric readouts
- simple controls with immediate feedback
- a short explanation of what is established physics, what is toy modeling, and what is speculative extension
- a path toward C++23 implementation with RAII, explicit ownership, tests, and later HPC/GPU optimization

Do not overbuild the first version. The first version should be small, correct, and inspectable.

## Preferred Implementation Stack

When building interactive simulators for this project, prefer:

- **C++23** for the simulation core
- **DirectX 12** for rendering and GPU-facing work
- **Dear ImGui** for tools, controls, live parameter editing, debug panels, and quick instrumentation

Do not default to a web app, Unity, Unreal, Python notebook, or generic visualization stack unless the user explicitly asks for that. The preferred shape is a native C++ sandbox where ImGui exposes the controls and diagnostics while DirectX 12 handles the rendered simulation view.

Good implementations should keep:

- simulation logic independent from ImGui widgets
- rendering code separate from physics kernels
- CPU reference paths before GPU optimization
- debug UI rich enough to inspect state, parameters, and errors
- GPU paths motivated by profiling or clear parallel structure

## Portfolio Lens

Every selected simulator should help tell this story:

> I learn physics by building small, rigorous sandbag models. I choose exercises that become reusable simulation primitives, compare them against real physics where useful, and engineer them cleanly enough to grow into a high-performance C++23 sandbox.

This is the guiding taste for the project.
