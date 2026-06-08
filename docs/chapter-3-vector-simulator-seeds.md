# Chapter 3 Vector Simulator Seeds

## Purpose

These are the first small simulator candidates derived from the Chapter 3 additional vector problems in Serway/Beichner. They are not one-to-one homework solutions. Each seed combines one or more questions into a reusable interactive instrument for the larger physics sandbox.

Selection follows `docs/simulation-selection-preferences.md`: prefer visual, animated, reusable, project-aligned simulators over isolated numeric answers.
Lesson writing follows `docs/lesson-authoring-guide.md`: build each simulator in small why-first chapters with diagrams, live controls, and clear recaps.

## 1. Local Frame Lab

Source questions:

- Problem 63: rotated coordinate system
- Problem 61: rectangular box face/body diagonals
- Problem 55: 3D vector components, magnitude, and axis angles

Concept:

Coordinate frames, components, rotations, basis vectors, projections, and 3D magnitude.

Sandbox role:

This becomes the first local-frame and coordinate-transform instrument. It supports later work on observer frames, tensor components, local metric frames, camera transforms, 3D grids, and GPU-space/world-space conversions.

Interactive controls:

- Select or drag a point/vector in a 3D lattice.
- Start constrained to the XY plane, then unlock XZ, YZ, and free 3D modes.
- Rotate the local coordinate axes with ImGui yaw/pitch/roll controls.
- Adjust 3D box dimensions `a`, `b`, and `c`.
- Toggle world basis, local basis, and projected components.

Live outputs:

- World coordinates `(x, y, z)` and local coordinates `(x', y', z')`.
- Rotation matrix or local basis vectors.
- Component arrows on each basis axis.
- 3D face diagonal and body diagonal magnitudes.
- Axis-angle readouts for the 3D vector mode.

Diagnostic:

The point/vector does not physically move when the coordinate frame rotates. Only its components change. This is the first visual lesson in "same object, different representation."

Extension:

Later this can grow into local tangent frames, camera/view transforms, tensor component visualization, and metric-aware coordinate displays.

Stage plan:

1. **XY Plane In A 3D World**: 3D lattice, world axes, interaction locked to XY, one selected vector, basic coordinate and magnitude readouts.
2. **Plane Slices**: add XY/XZ/YZ plane locks and projection shadows so 2D problems feel like slices through 3D.
3. **Free 3D Vector**: unlock full 3D placement, add depth guides, component legs, face diagonals, and body diagonal intuition.
4. **Rotated Local 3D Frame**: add local basis vectors `i'`, `j'`, `k'`, rotation controls, local components, and optional animated basis rotation.

## 2. Motion State Lab

Source questions:

- Problem 58: position vector as a function of time and `dP/dt`
- Problem 57: walking path resultant displacement
- Problem 62: displacement vector `r2 - r1`

Concept:

Position, displacement, velocity as derivative, path history, and state over time.

Sandbox role:

This becomes the first particle-motion kernel. It introduces the distinction between an object's current state, its path history, and its derivative. That matters later for particle integration, proper-time accumulation, and trajectories through fields.

Interactive controls:

- Animate a particle using a parameterized position function `P(t)`.
- Scrub time with a slider.
- Edit coefficients in `P(t)` using ImGui.
- Add piecewise path segments by clicking points.
- Toggle velocity arrows and displacement arrows.
- Pause, step, reset, and trace path history.

Live outputs:

- Current position vector.
- Velocity vector `dP/dt`.
- Total path length.
- Net displacement from start.
- Difference between instantaneous velocity and average displacement over an interval.

Diagnostic:

Velocity is the derivative of position, while displacement is only the difference between two positions. The path can be long and winding while the final displacement remains short.

Extension:

Later this becomes the base for particles moving through a time field, path-dependent proper time, integrator comparisons, and trajectory diagnostics.

## 3. Force Accumulator Lab

Source questions:

- Problem 56: sum of four force vectors
- Problem 51: resultant from multiple vectors
- Problem 52: solve coefficients so a vector combination cancels
- Problems 53-54: equal-magnitude vectors and the ratio of `|A+B|` to `|A-B|`

Concept:

Vector addition, resultant forces, cancellation, component decomposition, symmetry, and constructive/destructive vector combination.

Sandbox role:

This becomes the force-accumulation instrument for the sandbox. Later, gravity-like gradients, drag, EM-like forces, wind, user-applied forces, and vortex terms can all feed into the same accumulator.

Interactive controls:

- Add, remove, drag, and rotate force arrows.
- Set magnitudes numerically or by dragging arrow length.
- Toggle component decomposition.
- Toggle equal-magnitude two-vector mode for angle sweeps.
- Toggle "cancel resultant" mode that solves for a missing vector or coefficient.

Live outputs:

- Individual force components.
- Resultant vector.
- Net magnitude and direction.
- `|A+B|`, `|A-B|`, and their ratio in equal-vector mode.
- Warning/readout when the forces nearly cancel.

Diagnostic:

The object responds to the net vector, not to the visual complexity of the force diagram. Symmetric forces can cancel; nearly aligned forces reinforce; angle controls the strength of the resultant.

Extension:

Later this can become the visible debug panel for particle acceleration in the time-field simulator, including force budget breakdowns.

## 4. Medium-Relative Motion Lab

Source questions:

- Problem 59: jet velocity plus wind velocity
- Problem 57: path with multiple directed legs
- Problem 62: displacement between two positions

Concept:

Relative velocity, motion through a moving medium, observer frame vs. medium frame, and resultant ground motion.

Sandbox role:

This becomes a bridge from plain vector addition to frame-relative physics. It is especially useful for later analogue-gravity thinking, where excitations move relative to a medium whose own flow changes the effective path.

Interactive controls:

- Drag an aircraft velocity vector.
- Drag a wind/medium velocity vector.
- Animate the aircraft through a 2D space.
- Toggle "air-relative" and "ground-relative" trails.
- Add a spatially varying wind field as an optional advanced mode.
- Set target destination and show correction heading.

Live outputs:

- Air-relative velocity.
- Medium/wind velocity.
- Ground-relative velocity.
- Drift angle.
- Path traced in still air vs. moving medium.
- Arrival error relative to target.

Diagnostic:

The aircraft's motion relative to the ground is the vector sum of its motion through the air and the air's motion relative to the ground. Changing the frame changes the description, not the underlying update rule.

Extension:

Later this can become a flow-field navigation simulator, an analogue-spacetime intuition demo, or a prelude to particles moving through dynamic fields.

## 5. Iterative Attractor Map Lab

Source questions:

- Problem 60: treasure-map movement by fractions toward successive points
- Problems 53-54: parameter sweeps and ratios, as an optional mode
- Problem 62: displacement between current state and target

Concept:

Iterative state updates, interpolation toward targets, weighted movement, convergence, order effects, and attractor-like behavior.

Sandbox role:

This is the most "dynamic systems" Chapter 3 simulator. It turns simple vector movement into a state-update rule. That directly supports later relaxation solvers, gradient descent, field-following behavior, particle attractors, and iterative numerical methods.

Interactive controls:

- Place target points on a 2D map.
- Choose movement fractions such as `1/2`, `1/3`, `1/4`, `1/5`.
- Reorder target points and compare final locations.
- Animate each update step.
- Toggle repeated cycling through the targets.
- Add jitter/noise to the target points or movement fraction.

Live outputs:

- Current position.
- Target displacement vector.
- Step vector.
- Final position.
- Difference between path length and net displacement.
- Convergence trail when repeated cycling is enabled.

Diagnostic:

Each step is a vector interpolation between current state and target. The final point depends on update order unless the specific rule is order-independent. This makes state update rules visible and testable.

Extension:

Later this can become a relaxation-method visualizer, a gradient-descent toy, a particle-attractor sandbox, or an intro to iterative Poisson solvers.

## Recommended Build Order

1. **Motion State Lab**: establishes particle state, path tracing, animation, and derivative readouts.
2. **Force Accumulator Lab**: adds reusable vector arrows, components, resultant logic, and force debugging.
3. **Local Frame Lab**: adds rotated coordinates and later-ready frame transformation tools.
4. **Medium-Relative Motion Lab**: connects vector addition to frames and moving media.
5. **Iterative Attractor Map Lab**: introduces dynamic systems, interpolation, and solver-like update rules.

This order builds reusable infrastructure gradually: arrows, points, paths, animation, state, frames, and diagnostics.

## Shared Implementation Notes

Preferred stack:

- C++23 simulation core
- DirectX 12 renderer
- Dear ImGui controls and diagnostics

Shared primitives worth building once:

- `Vec2` and `Vec3` math utilities
- vector-arrow renderer
- draggable point handles
- path/trail renderer
- coordinate-grid renderer
- ImGui parameter panels
- deterministic step/pause/reset controls
- simple CSV or JSON snapshot export for diagnostics

Keep the first version of each simulator small. The goal is to make the concept move on screen, expose the useful numbers, and create a primitive that can be reused in the larger physics sandbox.
