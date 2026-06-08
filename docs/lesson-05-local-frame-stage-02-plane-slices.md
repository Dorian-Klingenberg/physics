# Lesson 05: Local Frame Lab Stage 2

## Stage Goal

Turn the fixed XY interaction from Stage 1 into selectable plane slices:

- XY, preserving the current `z`
- XZ, preserving the current `y`
- YZ, preserving the current `x`

The simulator is still constrained during editing, but changing plane orientation must not reinterpret or swap the coordinates of `P`.

## The Idea

Many early vector problems are really 2D problems. Stage 2 shows that a 2D problem can be understood as a slice through a 3D coordinate system.

```text
XY slice: P = (x, y, locked z)
XZ slice: P = (x, locked y, z)
YZ slice: P = (locked x, y, z)
```

The point `P` is still a 3D vector. The active plane only decides which two coordinates are editable. The third coordinate is preserved.

## The Formula

Clicking or dragging the scene casts a ray through the mouse position:

```text
R(t) = near + t direction
```

Each plane is a coordinate-aligned slice through the existing point:

```text
XY: Rz(t) = P.z
XZ: Ry(t) = P.y
YZ: Rx(t) = P.x
```

Solving for `t` is the same pattern in all three cases:

```text
t = (locked_axis_value - near_axis) / direction_axis
```

Then the simulator updates the two active coordinates and preserves the locked coordinate.

## What The User Can Do

Stage 2 controls:

- choose XY, XZ, or YZ,
- drag the two active coordinates with ImGui,
- click or drag the scene background to move `P` on the selected plane,
- reset `P`,
- toggle component legs,
- toggle depth hints.

Stage 2 readouts:

- full 3D point `P = (x, y, z)`,
- magnitude `|P| = sqrt(x^2 + y^2 + z^2)`,
- distance along each active coordinate,
- active slice and preserved coordinate.

## What Changed Visually

The renderer now draws the active grid plane instead of assuming XY forever.

The active plane gets:

- grid lines on the selected coordinate slice,
- a very light translucent plane fill,
- a brighter outline,
- a marked intersection where the preserved axis pierces the slice,
- the same vector `P`,
- component guide legs,
- measurement labels that follow whichever coordinates are active.

## What We Learned

Stage 2 creates the reusable plane-lock primitive:

- plane enum,
- plane-aware sliders,
- plane-aware click/drag picking,
- active-plane grid rendering,
- a light active-plane fill,
- preserved-axis intersection marker,
- labels that follow the selected slice,
- preserved-coordinate diagnostics.

This is the bridge from flat vector diagrams to actual 3D simulation space.

## Next Stage

Stage 3 unlocks free 3D vector placement.

That will remove the plane constraint and add stronger depth diagnostics: component legs along all three axes, face diagonals, and body diagonal intuition.
