# The Simulation-Physics Ladder: A Step-by-Step Build Plan for the Time Dilation Field Framework

## How to Use This Document

This roadmap runs two parallel tracks — a **simulation build track** and a **physics learning track** — organized into phases. Each phase pairs a working simulation milestone with the specific physics concepts needed to understand what the simulation is doing and why. The goal is never to finish all the physics before starting to build, and never to build blindly without knowing what questions to ask. The two tracks are interleaved by design.

Each simulation step is described in terms of what to build, what inputs and outputs look like, and what specific physics question the step is asking. Each physics step identifies the minimum knowledge target — not the entire subject, just the piece that directly connects to the simulation. Recommended resources are included for each phase.

***

## Phase 0 — The Substrate (Pre-Simulation Setup)

This phase is not optional. It establishes the numerical plumbing that everything else runs on. If the scalar field work already exists in the project, much of this is already done.

### Simulation 0: The Grid

Build a 2D scalar field \( F(x,y) \) on a grid. Store it as a flat 2D float array. Implement:

- **Sampling**: given a world-space position `(x, y)`, return the interpolated field value (bilinear interpolation)
- **Gradient**: return a 2D vector `(dF/dx, dF/dy)` computed by finite differences: \( \frac{\partial F}{\partial x} \approx \frac{F(x+h,y) - F(x-h,y)}{2h} \)
- **Visualization**: export as a grayscale texture, as a contour map, and as a vector arrow field

**What to test**: place a Gaussian "hill" in the field, verify that gradient arrows point away from the peak, verify interpolation is smooth at grid boundaries.

**Why it matters**: every subsequent simulation runs on this substrate. Bilinear interpolation and finite-difference gradients are the numerical atoms of the entire project.

### Physics 0: Derivatives, Gradients, and Fields

**Target concepts (not the full course — just these):**

- A scalar field assigns one number to every point in space. A vector field assigns one vector.
- The gradient \( \nabla F \) points in the direction of steepest increase of \( F \)
- Finite differences are the numerical approximation of derivatives: \( \frac{dF}{dx} \approx \frac{F(x+h) - F(x)}{h} \)
- Bilinear interpolation is the continuous version of reading between grid cells

**Minimum resources:**
- **3Blue1Brown, "Essence of Calculus" (YouTube, free)**: 12 videos on derivatives and integrals, starting visually. The series builds exactly the intuition needed here[^1]
- **Khan Academy, Multivariable Calculus — Gradient unit**: free, 15-20 minutes of video

**Milestone check**: can write the gradient estimator from scratch, describe in one sentence what the gradient of the time field means physically.

***

## Phase 1 — The Time Field and Proper Time

### Simulation 1A: Static Time Field with Clocks

Implement `TimeField2D` as a thin wrapper around the scalar field, where the value at each point is the local time rate \( T(x,y) \in (0, 2] \). Create a field with a slow-time "well" at the center (e.g., a Gaussian dip: \( T(x,y) = 1 - A \cdot e^{-r^2/\sigma^2} \)).

Place 20–50 stationary "clocks" scattered across the grid. Each clock accumulates:
\[ \tau_i \mathrel{+}= T(x_i, y_i) \cdot dt \]

At each simulation tick, advance the global clock by `dt = 0.01`. Render each clock with a visual indicator (size, hue, or hand angle) proportional to its accumulated \( \tau_i \).

**What to observe**: clocks near the center should tick slower. After 1,000 ticks, the clock at the center should have accumulated less proper time than clocks at the boundary. Measure the ratio.

**What to test**: swap a slow-time well for a fast-time peak. Clocks at peak should now lead.

### Simulation 1B: Particle Moving Through the Time Well

Add a particle with `(position, velocity, properTime)`. No forces yet — just kinematic time scaling:
\[ x_{n+1} = x_n + v \cdot T(x_n) \cdot dt \]

Launch the particle on a straight-line path through the center of the slow-time well. Plot both the global-time path and the particle's accumulated proper time as a function of global ticks.

**What to observe**: the particle slows down (in proper-time terms) when it passes through the well, then speeds back up after. Its odometer (proper time) accumulates less than the global clock during transit.

### Physics 1: Time Dilation Basics — Special and Gravitational

**Target concepts:**

- **Coordinate time vs. proper time**: coordinate time is the global "tick count"; proper time is what a clock physically accumulates along its path. The distinction \( d\tau = \sqrt{-g_{\mu\nu} dx^\mu dx^\nu} \) collapses to \( d\tau = T(x)\,dt \) in the scalar-field approximation.
- **Special relativistic time dilation**: a moving clock runs slow by factor \( \gamma = 1/\sqrt{1 - v^2/c^2} \). This is kinematic.
- **Gravitational time dilation**: a clock deeper in a gravitational well runs slow. The weak-field formula is \( \tau(r) \approx 1 + \Phi/c^2 \), where \( \Phi = -GM/r \) is the gravitational potential. This is the direct formula the simulation is mimicking[^2]
- **Twin paradox intuition**: two paths through spacetime accumulate different proper times; the path with more acceleration (more "bending") accumulates less.

**Minimum resources:**
- **Susskind, *The Theoretical Minimum: Special Relativity and Classical Field Theory*** (book + free YouTube lectures): Susskind and Hrabovsky's series is aimed at exactly this level — people who know algebra and calculus but are not physicists. The special relativity volume covers time dilation at the exact level needed here.[^3][^4][^5]
- **Carroll, *The Biggest Ideas in the Universe: Space, Time, and Motion*** (2022): explains time dilation with actual equations but without assuming graduate math. The Mindscape podcast episode "Secrets of Einstein's Equation" covers the weak-field formula explicitly.[^6][^7]

**Milestone check**: can state in one sentence why the particle accumulates less proper time through the well. Can write the Schwarzschild weak-field time-rate formula from memory.

***

## Phase 2 — Gradient Steering (Pseudo-Gravity)

### Simulation 2A: Gradient-Driven Acceleration

Implement the gradient-driven force law:
\[ \mathbf{a} = -k \, \nabla T \]

In code:
```csharp
Vector2 grad = timeField.Gradient(particle.Position);
particle.Velocity += -k * grad * dt;
particle.Position += particle.Velocity * dt;
```

Place a particle at rest 50 units from the slow-time well. Set \( k = 1.0 \). Observe: does the particle fall toward the center? Plot the path. Compare visually with what you would expect from Newtonian gravity.

**Experiment 2A-1 — vary k**: try \( k = 0.1 \), \( k = 1.0 \), \( k = 5.0 \). Observe different infall speeds. \( k \) is a coupling constant — the strength of the time-gradient force.

**Experiment 2A-2 — circular orbit**: give the particle a tangential initial velocity. Can it orbit the well? Adjust \( k \) and initial speed to find a stable orbit. Record the orbital period.

**Experiment 2A-3 — measure the force law**: at various distances \( r \) from the well center, measure the gradient magnitude \( |\nabla T| \). Plot vs. \( r \). Does it fall off as \( 1/r \), \( 1/r^2 \), or something else? The answer depends on the shape of \( T(x,y) \).

### Simulation 2B: Newtonian Comparison Harness

This is the most important diagnostic tool in the entire project. Run two simulations side-by-side from the same initial conditions:

1. **Newtonian**: \( \mathbf{a} = -GM/r^2 \cdot \hat{r} \)
2. **Time-gradient**: \( \mathbf{a} = -k\nabla T \) with Gaussian \( T \) field

Record and overlay:
- Orbit shapes (closed ellipse vs. precessing ellipse vs. open spiral)
- Orbital periods
- Energy conservation (does the total \( E = \frac{1}{2}mv^2 + V \) stay constant?)

**What to expect**: with a Gaussian \( T \) field, the force does not fall off as \( 1/r^2 \), so orbits will not be closed ellipses. This is the first major diagnostic. It raises the question: what density-to-time mapping would reproduce \( 1/r^2 \) fall-off?

### Physics 2: Classical Mechanics — Forces, Energy, and Orbits

**Target concepts:**

- **Newton's second law**: \( F = ma \), and how to integrate it numerically (Euler, Verlet, Runge-Kutta 4)
- **Conservative forces and potentials**: a force is conservative if it can be written as \( \mathbf{F} = -\nabla V \). The gradient-steering rule \( \mathbf{a} = -k\nabla T \) is conservative — \( T \) itself acts as the potential. This is the connection to energy conservation.
- **Orbital mechanics**: Kepler's laws, closed ellipses under inverse-square law, why a \( 1/r^2 \) force gives closed orbits (Bertrand's theorem) but most other force laws do not
- **Lagrangian mechanics (first encounter)**: the action \( S = \int L\,dt \), the Lagrangian \( L = T - V \), and the Euler-Lagrange equation that produces equations of motion. This is important because the simulation's equations of motion are precisely an Euler-Lagrange system with \( V = kT(x) \).

**Minimum resources:**
- **Susskind, *The Theoretical Minimum: Classical Mechanics*** (book + YouTube): covers Lagrangian mechanics, conservative forces, energy, and orbits starting from algebra and basic calculus. This is the right level for this phase.[^4][^8][^3]
- **Taylor, *Classical Mechanics*** (undergraduate textbook): if more depth is needed, Chapter 4 (energy and potentials) and Chapter 6 (calculus of variations / Lagrangian) are the target sections[^9][^10]
- **Runge-Kutta 4 (any numerical methods reference)**: the particle integrator needs to upgrade from Euler to RK4 before orbital comparisons are meaningful[^11][^12]

**Milestone check**: can explain why the Gaussian time field does not produce closed orbits. Can derive what \( T(r) \) would need to look like to produce \( 1/r^2 \) fall-off from \( \mathbf{a} = -\nabla T \). (Hint: \( T(r) = -C/r + \text{const} \) — a Newtonian potential shape.)

***

## Phase 3 — Matter Generates the Field

### Simulation 3A: Matter Density to Time Field

Add a matter-density layer \( \rho(x,y) \). Generate the time field from density using the exponential model:
\[ T(x,y) = e^{-a\,\rho(x,y)} \]
Or the weak-field GR approximation:
\[ T(x,y) \approx 1 + \Phi(x,y)/c^2 \]
where \( \Phi \) is computed from \( \rho \) using Poisson's equation:
\[ \nabla^2 \Phi = 4\pi G \rho \]

Implement Poisson's equation solver on the grid (iterative relaxation or FFT-based). Now \( \rho \to \Phi \to T \to \mathbf{a} \) is a complete pipeline.

**Experiment 3A-1**: place a single point mass (Gaussian density blob). Verify the resulting \( T(x,y) \) field has the expected shape (deepest well at center, gradual rise outward). Verify the gradient force law still applies.

**Experiment 3A-2**: compare the gradient field from Poisson-sourced \( T \) against direct Newtonian \( 1/r^2 \). They should now agree much more closely, because the Poisson equation is the same one that underlies Newtonian gravity.

**Experiment 3A-3**: place two density blobs and let test particles move in the combined field. Do the particles fall toward both blobs? Can you produce a stable binary orbit?

### Simulation 3B: Self-Consistent Update (Optional but Powerful)

Allow particles to contribute to \( \rho \). Each particle updates the density field at its location; the density field updates \( T \); \( T \) updates forces on all particles. This is a full self-consistent N-body time-field simulation. Implement with a small timestep and verify energy conservation.

### Physics 3: Gravitational Field Theory and Poisson's Equation

**Target concepts:**

- **Gravitational potential \( \Phi \)**: the scalar field satisfying \( \mathbf{g} = -\nabla\Phi \) and Poisson's equation \( \nabla^2\Phi = 4\pi G\rho \). This is the bridge equation: it connects matter density directly to the gradient field that drives acceleration.
- **The Laplacian \( \nabla^2 \)**: the divergence of the gradient, measuring "how much the field at a point differs from its surroundings." In flat space and radial symmetry, \( \nabla^2(1/r) = -4\pi\delta^3(\mathbf{r}) \) — a point mass is a delta function source for the potential.
- **Weak-field GR**: in the weak-field, slow-motion limit, GR reduces to Newtonian gravity plus a time-rate correction. The metric time component \( g_{00} \approx -(1 + 2\Phi/c^2) \) gives \( \tau(r) \approx 1 + \Phi/c^2 \). The project's \( T(x,y) \) is this correction term.
- **Iterative Poisson solvers**: the simplest approach is the Gauss-Seidel relaxation method or a Jacobi iteration — both are O(N²) per timestep but easy to implement on a grid.

**Minimum resources:**
- **Susskind, *General Relativity: The Theoretical Minimum*** (YouTube, free): the weak-field limit and the connection to Newtonian potential are covered directly[^5][^13]
- **Hartle, *Gravity: An Introduction to Einstein's General Relativity***: uses a "physics-first" approach that starts with observational phenomena (time dilation, light bending) and works backward to the metric. The best undergraduate GR text for the weak-field stage[^14][^15][^16]
- **Schutz, *A First Course in General Relativity*** (3rd ed.): if Hartle feels too applied, Schutz builds the math (tensors, differential geometry) more carefully. Review by Eric Poisson in *Classical and Quantum Gravity* calls it one of the best introductory GR books ever written[^17][^18][^19][^20]

**Milestone check**: can write Poisson's equation, explain what it connects, and implement it numerically on the grid. Can explain in one paragraph why the weak-field GR time rate and the Newtonian potential are the same thing (up to a \( 1/c^2 \) factor).

***

## Phase 4 — Wave Propagation in the Time Field

### Simulation 4A: Static Field Perturbations and Wave Modes

Allow \( T(x,y) \) to evolve over time. Implement a wave equation for the time field:
\[ \frac{\partial^2 T}{\partial t^2} = c_T^2 \nabla^2 T + S(x,y,t) \]
where \( S \) is a source term (e.g., a particle creating a disturbance). Use finite differences in both space and time.[^21]

**Experiment 4A-1**: start with a small Gaussian perturbation in \( T \) and watch it radiate outward. Measure the wave speed. Compare to the coupling constant \( c_T \).

**Experiment 4A-2**: let two perturbations interfere. Observe constructive and destructive interference. This is the first "field interaction" experiment in the project.

**Experiment 4A-3**: does a moving massive particle leave a "wake" in \( T \)? Implement a particle that updates \( \rho \), which sources \( T \) via Poisson, and watch whether a disturbance in \( T \) propagates outward. This is a toy analog of gravitational waves.

### Physics 4: Waves, PDEs, and Field Dynamics

**Target concepts:**

- **The wave equation**: \( \partial^2 u/\partial t^2 = c^2 \nabla^2 u \). Solutions are propagating waves with speed \( c \). The dispersion relation: \( \omega^2 = c^2 k^2 \).
- **The diffusion equation vs. the wave equation**: diffusion smooths field values over time (all Fourier modes decay); the wave equation preserves them (modes oscillate). Knowing which equation governs \( T \) changes what kind of physics you get.
- **Superposition principle**: for linear field equations, disturbances add. Understanding why the simulation's wave modes interfere the way they do requires this.
- **Gravitational waves (conceptual)**: in GR, perturbations to the metric propagate as waves at speed \( c \). The linearized Einstein equations give the wave equation for metric perturbations \( h_{\mu\nu} \). The simulation's toy wave equation is a scalar analog of this.

**Minimum resources:**
- **Carroll, *Spacetime and Geometry***: Chapter 6 covers linearized GR and gravitational waves. Described as "the best textbook on contemporary gravitational physics for beginning graduate students"[^22][^23][^24][^25]
- **MIT OCW 8.962 (General Relativity, Spring 2020)**: free lecture notes and problem sets; uses Carroll as the primary text[^25][^13]
- **Any CFD/PDE numerical methods resource**: finite-difference wave equation implementation with stability analysis (CFL condition)[^26][^21]

***

## Phase 5 — Metric and Geodesic Upgrade

At this phase, the scalar field model has been pushed to its limit. The project is ready to understand why full GR uses a tensor rather than a scalar.

### Simulation 5A: 2D Metric Simulation (Schwarzschild Slice)

Instead of a scalar \( T(x,y) \), implement a 2D diagonal metric \( g_{\mu\nu} \) as two scalar fields: \( g_{00}(x,y) \) and \( g_{11}(x,y) = g_{22}(x,y) \). The Schwarzschild metric in 2D slice:
\[ g_{00}(r) = -(1 - 2GM/rc^2), \quad g_{rr}(r) = (1 - 2GM/rc^2)^{-1} \]

Compute Christoffel symbols \( \Gamma^\mu_{\ \alpha\beta} \) numerically from the metric components. Integrate the geodesic equation:
\[ \frac{d^2 x^\mu}{d\tau^2} + \Gamma^\mu_{\ \alpha\beta} \frac{dx^\alpha}{d\tau} \frac{dx^\beta}{d\tau} = 0 \]

Compare geodesic orbits against both the scalar time-field orbits from Phase 2 and Newtonian orbits. Observe perihelion precession as a natural output.

### Simulation 5B: Light Bending Test

Send a null geodesic (photon path, \( ds^2 = 0 \)) past the Schwarzschild metric. Measure the deflection angle. Compare to:
- Scalar (Nordström) prediction: zero deflection
- GR prediction: \( \delta\phi = 4GM/rc^2 \)
- Simulation output

**What this reveals**: the scalar time-rate model bends the photon path by approximately half the GR amount — exactly the Nordström failure mode identified in Phase 0's literature. This is where the simulation "finds its own limit" as a research result.

### Physics 5: Tensors, the Metric, and Geodesics

**Target concepts:**

- **Tensors**: objects that transform predictably under coordinate changes. The metric tensor \( g_{\mu\nu} \) is a \( (0,2) \) tensor that measures distances and angles. Contravariant vs. covariant components. Index raising/lowering.
- **The interval**: \( ds^2 = g_{\mu\nu} dx^\mu dx^\nu \). In flat spacetime this is \( -c^2dt^2 + dx^2 + dy^2 + dz^2 \). Geodesics extremize the proper time between events.
- **Christoffel symbols**: encode how the metric changes from point to point. They are not tensors themselves but are computed from metric derivatives: \( \Gamma^\mu_{\ \alpha\beta} = \frac{1}{2} g^{\mu\nu} (\partial_\alpha g_{\nu\beta} + \partial_\beta g_{\nu\alpha} - \partial_\nu g_{\alpha\beta}) \)
- **The geodesic equation**: the curved-spacetime version of Newton's first law. Free-fall is motion along a geodesic.
- **The Riemann curvature tensor**: measures how much two geodesics initially parallel converge or diverge. The Einstein tensor is a contraction of the Riemann tensor.

**Minimum resources:**
- **3Blue1Brown, "Essence of Linear Algebra" (YouTube, free)**: 16 videos on vectors, matrices, transformations, and eigenvectors — the foundation for understanding tensors[^27][^28][^29][^30]
- **Schutz, *A First Course in General Relativity***: Chapters 2–6 build tensors from scratch in flat spacetime, then extend to curved spacetime. Written for students with only calculus and vectors[^18][^19][^20]
- **MIT OCW 8.962 Lecture Notes on Tensor Calculus** (free PDF: "Introduction to Tensor Calculus for General Relativity" by Edmund Bertschinger)[^31]

**Milestone check**: can write out the Christoffel symbol formula, compute one by hand from a given metric, and implement the geodesic ODE in code. Can explain in two sentences why the scalar model misses half the light bending.

***

## Phase 6 — Vector and Directional Time Fields

With the tensor metric baseline established, the project can now sensibly ask: what does the "3+3 directional time model" mean mathematically, and what does it add?

### Simulation 6A: Time Dilation with Spin / Vorticity

Implement a vortex-like time field:
\[ T(x,y) = T_0 - A \cdot e^{-r^2/\sigma^2}, \quad \Omega(x,y) = \omega_0 \cdot e^{-r^2/\sigma^2} \]
where \( \Omega \) is a rotation rate. The equations of motion become:
\[ \mathbf{a} = -k\nabla T + \ell \cdot (\Omega \times \mathbf{v}) \]
The second term is Coriolis-like — it deflects the particle tangentially. This is a toy analog of frame dragging (the Kerr metric's off-diagonal \( g_{t\phi} \) component).

**What to observe**: orbits precess in the direction of field rotation. Particles starting at rest are set in circular motion even without a radial kick. This is qualitatively what Lense-Thirring frame dragging produces.

### Physics 6: Torsion, Spin Connections, and the Vierbein Formalism

**Target concepts:**

- **The vierbein (tetrad)**: a set of four orthonormal vectors at each spacetime point. The local time-rate \( T(x) \) appears as the \( e^0_{\ 0} \) component of the vierbein. Making it directional — a "time dilation vector" — corresponds to allowing the vierbein to have non-trivial orientations.
- **The spin connection**: encodes how the local frame rotates from point to point, coupling to spin-1/2 particles via the Dirac equation in curved spacetime.
- **Torsion and Einstein-Cartan theory**: torsion is the antisymmetric part of the connection. It couples to intrinsic spin. A time field with vortex structure generates effective torsion — the mathematical object for encoding angular-momentum-like structure in geometry.
- **Frame dragging**: in the Kerr metric, off-diagonal metric components \( g_{t\phi} \) cause orbits to precess even in "empty" space around a rotating mass.

**Minimum resources:**
- **Carroll, *Spacetime and Geometry*, Chapter 3**: covers connections, parallel transport, and geodesics. The spin connection appears in the vierbein formalism appendix[^23][^22]
- **Schutz, Chapter 6 on covariant derivatives and curvature**: the rigorous mathematical treatment[^19][^20]

***

## The Complete Phase Map

| Phase | Simulation Build | Physics To Learn | Key Question |
|---|---|---|---|
| 0 | 2D scalar field, gradient, visualization | Derivatives, gradients, finite differences | Does the grid implement the math correctly? |
| 1 | Static time field, clocks, particle proper time | SR and GR time dilation, proper time | Does a particle accumulate less proper time in the well? |
| 2 | Gradient-driven force, orbits, Newtonian comparison | Classical mechanics, Lagrangians, orbits | Does \( \mathbf{a} = -k\nabla T \) reproduce gravity-like behavior? |
| 3 | Density → potential → time field (Poisson) | Gravitational potential, Poisson's equation, weak-field GR | Can matter density generate the right time field to recover \( 1/r^2 \)? |
| 4 | Dynamic wave propagation in \( T \) | Wave equation, PDEs, linearized gravity | Do field disturbances propagate? Do they look like gravitational waves? |
| 5 | Full 2D metric, Christoffel symbols, geodesics | Tensors, metric geometry, Schwarzschild solution | Does the scalar model fail on light bending? By exactly how much? |
| 6 | Vortex time field, frame-drag analogs, spin structures | Vierbein, torsion, spin connection | Can directional time structure encode spin/angular momentum? |

***

## Learning Resource Stack by Phase

The resources below are ordered from most accessible to most rigorous within each category. No phase requires reading an entire resource — only the sections that directly support the simulation question.

### Visual / Conceptual Foundations (All Phases)

- **3Blue1Brown, "Essence of Calculus"** — YouTube, free: 12 videos, builds derivatives and integrals from visual geometry[^1]
- **3Blue1Brown, "Essence of Linear Algebra"** — YouTube, free: 16 videos, builds vectors, matrices, transformations — prerequisite for tensors[^28][^30][^27]
- **Carroll, *The Biggest Ideas in the Universe: Space, Time, and Motion*** (2022): physics with equations but without graduate math prerequisites[^32][^7][^33]

### Classical Mechanics (Phases 0–3)

- **Susskind, *The Theoretical Minimum: Classical Mechanics*** — book + YouTube Stanford lectures: designed for self-taught learners, covers Lagrangians, Hamiltonians, and phase space[^8][^3][^4][^5]
- **Taylor, *Classical Mechanics*** — undergraduate textbook: excellent on energy, orbits, Lagrangian/Hamiltonian methods, calculus of variations[^10][^9]

### Special Relativity (Phase 1)

- **Susskind, *The Theoretical Minimum: Special Relativity and Classical Field Theory*** — book + YouTube: builds SR from first principles, covers time dilation rigorously with equations[^3][^5]
- **MIT OCW 8.20 (Special Relativity)** — free online

### General Relativity (Phases 3–6)

- **Hartle, *Gravity: An Introduction to Einstein's General Relativity*** — "physics-first" approach, starts with phenomena: includes free solutions manual; rated "probably the most accessible serious introduction to GR"[^15][^16][^14]
- **Schutz, *A First Course in General Relativity*** (3rd ed.) — "math-first" approach, builds tensors carefully: ideal if the Lagrangian/tensor track is preferred[^20][^17][^18][^19]
- **Carroll, *Spacetime and Geometry*** — graduate level, used in MIT 8.962: the transition textbook from undergraduate to research-grade GR[^24][^22][^23][^25]
- **MIT OCW 8.962 (General Relativity, Spring 2020)** — free lectures + problem sets, uses Carroll[^13][^25]
- **Susskind GR Theoretical Minimum (YouTube)** — free Stanford continuing studies lectures: follows the Theoretical Minimum philosophy[^5]

### Numerical Methods (All Simulation Phases)

- **University of Maryland, "Numerical Integration with Examples"** — free web resource: covers Euler, Euler-Cromer, RK4 with worked code examples[^34][^11]
- **Particle-in-Cell Consulting, "The Finite Difference Method"** — free tutorial: covers discretization and finite differences on grids[^35]
- **Susan Fowler, "So You Want to Learn Physics..."** — blog post with complete curriculum ordering: a widely shared self-study guide that covers the full undergraduate-to-graduate path[^36][^37][^38][^39]

***

## The Simulation-Physics Feedback Loop

The most important thing to internalize about this roadmap: **the simulations are not illustrations of the physics. They are instruments for finding where the physics breaks.** Each simulation should be run until it produces a failure — a place where the toy model diverges from known behavior. That divergence is the research target. The physics knowledge for each phase is not background reading; it is the tool for diagnosing the specific failure the simulation just revealed.

This is exactly the methodology that produced Nordström's scalar gravity (failed on light bending), Brans-Dicke theory (extended it with a scalar field), and eventually GR (replaced the scalar with a full tensor). The project is running that same path in simulation space. The physics curriculum is the map for reading the instrument's output.

A practical discipline: for every simulation step, write a one-paragraph "diagnostic summary" that states (a) what the simulation produced, (b) how it compares to the established physics target, and (c) what specific piece of physics knowledge would explain the discrepancy. That paragraph is the connection between the two tracks.

---

## References

1. [Video Review: 3Blue1Brown - Cross Trained Mind](https://crosstrainedmind.com/2021/02/26/video-review-3blue1brown/) - YouTube Channel: 3Blue1BrownHost: Grant Sanderson Summary This YouTube channel is a great resource t...

2. [How To Learn General Relativity: A Step-By-Step Guide](https://profoundphysics.com/how-to-learn-general-relativity-a-step-by-step-guide/) - A complete step-by-step guide to learn both the special and general theories of relativity without y...

3. [The Theoretical Minimum: What You Need to Know to Start Doing ...](https://books.google.com/books/about/The_Theoretical_Minimum.html?id=dgicAgAAQBAJ) - A master physicist and a self-taught scientist present the ultimate introduction to classical mechan...

4. [The Theoretical Minimum - Wikipedia](https://en.wikipedia.org/wiki/The_Theoretical_Minimum) - The Theoretical Minimum: What You Need to Know to Start Doing Physics is a popular science book by L...

5. [The Theoretical Minimum |](https://theoreticalminimum.com) - These courses collectively teach everything required to gain a basic understanding of each area of m...

6. [211 | Solo: Secrets of Einstein's Equation - Sean Carroll](https://www.preposterousuniverse.com/podcast/2022/09/19/211-solo-secrets-of-einsteins-equation/)

7. [Sean Carroll’s Biggest Ideas](https://podcasts.apple.com/us/podcast/sean-carrolls-biggest-ideas/id1169885840?i=1000580056282) - Podcast Episode · Into the Impossible With Brian Keating · September 20, 2022 · 1h 28m

8. [The Theoretical Minimum](https://www.madscitech.org/tm/) - The Theoretical Minimum is a book for anyone who has ever regretted not taking physics in college—or...

9. [Excellent Classical Mechanics Book for Self-Study - YouTube](https://www.youtube.com/watch?v=yU2E5CUoVcE) - In this video, I review the book Classical Mechanics by John R. Taylor. I would highly recommend thi...

10. [taylor-2005-classical-mechanics.pdf - Slideshare](https://www.slideshare.net/slideshow/taylor2005classicalmechanicspdf/264339704) - The document is an extensive compilation of classical mechanics principles, including detailed expla...

11. [Numerical integration, with examples](https://www.physics.umd.edu/hep/drew/numerical_integration.html)

12. [9. Numerical integration — AE1205: Programming and Scientific ...](https://ae1205.lr.tudelft.nl/reader/content/9-numerical-integration.html)

13. [General Relativity | Physics - MIT OpenCourseWare](https://opencw.aprende.org/courses/physics/8-962-general-relativity-spring-2006/) - 8.962 is MIT's graduate course in general relativity, which covers the basic principles of Einstein'...

14. [Gravity An Introduction to Einstein's General Relativity](https://www.cambridge.org/highereducation/books/gravity/F9085ABB699F7A3A05ADD0B3930D98E0) - This ground-breaking text helped to bring general relativity into the undergraduate curriculum, maki...

15. [Gravity: An Introduction To Einstein'S General Relativity - Hartle](https://books.google.com/books/about/Gravity_An_Introduction_To_Einstein_S_Ge.html?id=Uwgd3U5ebOAC) - The Aim Of This Ground-Breaking New Text Is To Bring General Relativity Into The Undergraduate Curri...

16. [Reviews of “Gravity: An Introduction to Einstein's General Relativity”](https://www.physicsforums.com/threads/reviews-of-gravity-an-introduction-to-einsteins-general-relativity.1062323/) - It's probably the most accessible serious introduction to GR. And possibly the best for self-study. ...

17. [BOOK REVIEW: A First Course in General Relativity (Second Edition ...](http://ui.adsabs.harvard.edu/abs/2010CQGra..27j9001P/abstract) - The revisions bring the book up-to-date, and they ensure that Schutz's text will remain in the panth...

18. [Any recommendations for self studying regarding general relativity?](https://www.reddit.com/r/AskPhysics/comments/onvjdp/any_recommendations_for_self_studying_regarding/) - schutz begins with special relativity, and works on making it super buff with the mathematics you wi...

19. [A First Course in General Relativity | Cambridge Aspire website](https://www.cambridge.org/highereducation/books/a-first-course-in-general-relativity/933C1A8C84420EF1349B676F0D70A065) - This widely used textbook to provide the first step into general relativity for advanced undergradua...

20. [A First Course in General Relativity - Goodreads](https://www.goodreads.com/book/show/3973649) - The approach of the book is pedagogically excellent: after a good speedy review of the main concepts...

21. [Wave Simulation from scratch using finite difference method](https://www.youtube.com/watch?v=4IL8n8yYNjw) - CORRECTION at 6:16, the equation should end with dt^2 * s(x,t). CORRECTION at 3:48, the delta functi...

22. [Spacetime and Geometry](https://www.cambridge.org/id/universitypress/subjects/physics/cosmology-relativity-and-gravitation/spacetime-and-geometry-introduction-general-relativity)

23. [Spacetime and Geometry - Wikipedia](https://en.wikipedia.org/wiki/Spacetime_and_Geometry)

24. [Spacetime and Geometry | Cosmology, relativity and gravitation](https://www.cambridge.org/us/universitypress/subjects/physics/cosmology-relativity-and-gravitation/spacetime-and-geometry-introduction-general-relativity?format=HB&isbn=9781108488396)

25. [Readings | General Relativity | Physics](https://ocw.mit.edu/courses/8-962-general-relativity-spring-2020/pages/readings/) - This section lists additional text suggested for the course.

26. [4. Introduction to Finite-Difference Methods for Computational Fluid ...](https://www.youtube.com/watch?v=sMqbVc0UHac) - This is the 4th lecture in a series of 20 on Computational Fluid Dynamics (CFD). The topic is an int...

27. [Essence of Linear Algebra - 3Blue1Brown Classic Math Teaching ...](https://www.aibars.net/en/library/open-learning-ai/details/728737898546991104) - "Essence of Linear Algebra" by 3Blue1Brown is the world's most popular linear algebra teaching video...

28. [3Blue1Brown: Essence of Linear Algebra - Manning Publications](https://www.manning.com/livevideo/3blue1brown-essence-of-linear-algebra) - In this liveVideo course mathematician Grant Sanderson—better known on YouTube as 3blue1brown—lays o...

29. [[Linear Algebra, Calculus] I HIGHLY recommend 3Blue1Brown's Youtube channel for help learning.](https://www.reddit.com/r/learnmath/comments/7hziip/linear_algebra_calculus_i_highly_recommend/)

30. [Essence of linear algebra preview - 3Blue1Brown](https://www.3blue1brown.com/lessons/eola-preview/) - The introduction to a series on visualizing core ideas of linear algebra.

31. [Introduction to Tensor Calculus for General Relativity](https://web.mit.edu/edbert/GR/gr1.pdf)

32. [Sean Carroll: Understanding Space, Time and Motion](https://podcasts.apple.com/us/podcast/sean-carroll-understanding-space-time-and-motion/id1589146361?i=1000581076205) - Podcast Episode · Commonwealth Club of California Podcast · September 29, 2022 · 1h 8m

33. [Sean Carroll, "The Biggest Ideas in the Universe" : CSPAN2](https://archive.org/details/CSPAN2_20230326_180000_Sean_Carroll_The_Biggest_Ideas_in_the_Universe) - Theoretical physicist Sean Carroll broke down some of the complex equations and ideas in physics for...

34. [Euler-Cromer method](https://physics.umd.edu/hep/drew/numerical_integration/)

35. [The Finite Difference Method - Particle in Cell Consulting, LLC](https://www.particleincell.com/2011/finite-difference-method/) - The Finite Difference Method (FDM) is a way to solve differential equations numerically. It is not t...

36. [So You Want to Learn Physics... - Susan Fowler](https://www.susanjfowler.com/blog/2016/8/13/so-you-want-to-learn-physics) - ... (linear algebra, complex analysis, real analysis, partial differential equations, and ordinary d...

37. [Physics — Susan Rigetti](https://www.susanrigetti.com/physics) - A comprehensive and accessible curriculum that has helped hundreds of thousands of people learn phys...

38. [So You Want To Learn Physics... (57 books) - Goodreads](https://www.goodreads.com/list/show/159952.So_You_Want_To_Learn_Physics_) - 57 books — This list was created and voted on by Goodreads members. A reading list by Susan Fowler f...

39. ["So you want to learn physics...", an extraordinary blog post by ...](https://www.reddit.com/r/Physics/comments/4ydkxg/so_you_want_to_learn_physics_an_extraordinary/) - Bravo to Susan J. Fowler and those like her who are inspired to want to really, really learn and und...

