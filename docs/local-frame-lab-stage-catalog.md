# Local Frame Lab Stage Catalog

## Purpose

This catalog preserves the intended build path for the first Chapter 3 simulator. The Local Frame Lab should be built as a 3D simulator from the start, but with 2D training wheels so the lesson stays approachable.

Each stage should be completed, documented, and kept buildable before moving to the next stage.

## Source Problems

- Serway/Beichner Chapter 3, Problem 63: rotated coordinate system
- Serway/Beichner Chapter 3, Problem 61: rectangular box face/body diagonals
- Serway/Beichner Chapter 3, Problem 55: 3D vector components, magnitude, and axis angles

## Stage 1: XY Plane In A 3D World

Lesson:

- `docs/lesson-04-local-frame-stage-01-xy-plane-in-3d.md`

Core idea:

The selected vector is the object. Coordinates are a description of that object.

Build:

- 3D lattice renderer with quiet depth cues
- world X/Y/Z axes
- interaction locked to XY plane
- one selected point/vector `P`
- ImGui controls for reset and direct component editing
- readouts for `(x, y, z)`, magnitude, and active plane

Reusable primitive:

- 3D grid/lattice rendering
- selected vector state
- plane-constrained interaction

## Stage 2: Plane Slices

Lesson:

- `docs/lesson-05-local-frame-stage-02-plane-slices.md`

Core idea:

2D vector problems are constrained slices through 3D space.

Build:

- XY, XZ, and YZ plane-lock modes
- visible active plane highlight
- projection shadows onto the active plane
- readouts that make the locked coordinate explicit

Reusable primitive:

- plane selection
- projection-to-plane diagnostics
- coordinate slice controls

## Stage 3: Free 3D Vector

Core idea:

Magnitude, components, and diagonals generalize naturally from 2D to 3D.

Build:

- free 3D point/vector placement
- camera/orbit controls if needed
- component legs along X, Y, and Z
- optional box dimensions `a`, `b`, and `c`
- face diagonal and body diagonal visualization
- axis-angle readouts

Reusable primitive:

- free 3D vector editing
- component-leg rendering
- diagonal/magnitude diagnostics
- camera controls for later 3D simulators

## Stage 4: Rotated Local 3D Frame

Core idea:

The vector can stay fixed while its components change under a different basis.

Build:

- local basis vectors `i'`, `j'`, and `k'`
- yaw/pitch/roll or equivalent rotation controls
- local coordinate readouts `(x', y', z')`
- component projections onto the rotated basis
- optional animated basis rotation
- formula display for projection onto basis vectors

Reusable primitive:

- local frame representation
- basis-vector visualization
- coordinate transform diagnostics
- groundwork for camera transforms, field frames, tensors, and relativity-adjacent observer frames

## Preservation Rule

Before starting the next stage:

- update the stage lesson
- verify the app builds
- keep the previous stage narrative intact
- preserve a code snapshot or build target when the next stage will substantially change the implementation
- record the new controls, formulas, and visual diagnostics
