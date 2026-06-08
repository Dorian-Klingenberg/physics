# Selective Physics Curriculum for the Simulation Sandbox

## Purpose

This curriculum turns the existing simulation roadmap into a targeted reading path. The goal is not to complete a full undergraduate physics text. The goal is to learn exactly enough physics to build, test, and explain a portfolio-grade C++23 simulation sandbox inspired by relativity, quantum mechanics, fields, waves, and dynamic systems.

Use *The Art of Modeling Dynamic Systems* as the straight-through modeling spine. Read it right through. Use Serway and Beichner's *Physics for Scientists and Engineers*, fifth edition, as a selective calibration text: read the chapters that directly support the simulations, skim chapters that fill gaps, and skip chapters that are not needed for the first portfolio arc.

The working posture:

> Learn difficult physics by building it. Keep the models playful, inspectable, and breakable. Compare against real physics where useful, but do not frame the project as a scientific proof.

## Book Roles

### The Art of Modeling Dynamic Systems

Read this straight through.

This book is the modeling-method spine for the project. Use it for:

- dynamic systems thinking
- model boundaries and assumptions
- deterministic vs. chaotic behavior
- feedback loops
- state variables
- simulation diagnostics
- interpreting model failure without treating it as embarrassment

For every simulation phase, ask Morrison-style questions:

1. What is the system state?
2. What rules update the state?
3. What feedback loops exist?
4. What assumptions make the model simpler than reality?
5. What output would reveal that the model is useful or broken?

### Physics for Scientists and Engineers

Use this selectively.

This book is the physics-calibration source. It is not the project spine. Since you already have basic first-year Newtonian physics, do not spend equal time on every chapter. Use the book to strengthen the math and concepts that map directly to the simulations.

Assumption: this is the Serway/Beichner fifth edition with chapters 1-46.

## Serway/Beichner Priority Path

### Read Carefully

These chapters are directly tied to simulations in the current roadmap.

| Chapter | Topic | Why It Matters |
|---|---|---|
| 3 | Vectors | Needed for fields, gradients, particle motion, components, directions, and coordinate thinking. |
| 6 | Circular Motion and Other Applications of Newton's Laws | Needed for orbits, centripetal acceleration, rotating fields, and vortex/frame-drag-inspired toys. |
| 7 | Energy and Energy Transfer | Needed for work, kinetic energy, energy diagnostics, and conservation checks. |
| 8 | Potential Energy | Needed for conservative forces, gradient steering, and potential-field intuition. |
| 13 | Universal Gravitation | Core chapter for inverse-square gravity, gravitational potential, orbit comparison, and weak-field intuition. |
| 15 | Oscillatory Motion | Needed for oscillators, restoring forces, stability, periodic response, and field modes. |
| 16 | Wave Motion | Needed for propagating field disturbances and finite-difference wave experiments. |
| 18 | Superposition and Standing Waves | Needed for interference, modes, resonance, and pattern formation. |
| 34 | Electromagnetic Waves | Useful for wave-speed analogies, propagating fields, and field-energy intuition. |
| 39 | Relativity | Essential for proper time, time dilation, relativistic calibration, and portfolio credibility. |
| 40 | Introduction to Quantum Physics | Needed for quantum-inspired models, wave-particle ideas, and modern-physics framing. |
| 41 | Quantum Mechanics | Needed for wavefunctions, probability amplitudes, interference, and simple state evolution. |
| 43 | Molecules and Solids | Useful for material analogs, lattices, quasiparticles, and condensed-matter intuition. |

### Skim or Use as Reference

These chapters are useful, but should not slow down the first build arc.

| Chapter | Topic | How to Use It |
|---|---|---|
| 1 | Physics and Measurements | Skim dimensional analysis, units, uncertainty, and order-of-magnitude estimation. |
| 2 | Motion in One Dimension | Skim if examples expose math gaps. |
| 4 | Motion in Two Dimensions | Skim projectile and vector motion examples. |
| 5 | The Laws of Motion | Skim as a refresher on Newtonian force models. |
| 9 | Linear Momentum and Collisions | Reference later for particle systems and conservation diagnostics. |
| 10 | Rotation of a Rigid Object About a Fixed Axis | Reference for rotation-heavy simulations. |
| 11 | Angular Momentum | Reference for spin, vortex, and frame-drag-inspired models. |
| 14 | Fluid Mechanics | Reference later for analogue-gravity and medium-flow intuition. |
| 17 | Sound Waves | Skim if building acoustic or medium analogs. |
| 23 | Electric Fields | Reference for field-line thinking and inverse-square analogies. |
| 24 | Gauss's Law | Reference for flux, divergence, and field-source relationships. |
| 25 | Electrical Potential | Reference for potential-field math and gradient relationships. |
| 29 | Magnetic Fields | Reference for vector fields and direction-dependent force behavior. |
| 30 | Sources of the Magnetic Field | Reference for circulation, curl-like intuition, and field generation. |
| 31 | Faraday's Law | Reference for induction-like toy models and changing fields. |
| Appendix B | Math Review | Use whenever calculus, trig, vector algebra, or notation slows you down. |

### Deprioritize Unless Curiosity Pulls You There

These are not central to the first portfolio arc.

| Chapter Range | Topic | Reason |
|---|---|---|
| 19-22 | Thermal physics | Useful later for entropy/statistical models, but not central now. |
| 26-28 | Capacitance, current, circuits | Not central unless building electrical analogs. |
| 32-33 | Inductance and AC circuits | Optional unless circuit-style resonant systems become important. |
| 35-38 | Optics | Useful for ray/path bending, but lower priority than waves and relativity. |
| 42 | Atomic Physics | Interesting, but not needed before the first quantum-inspired experiments. |
| 44-46 | Nuclear, particle physics, cosmology | Interesting background, but not necessary for the first simulation portfolio arc. |

## Map to Simulation Milestones

| Simulation Milestone | Serway/Beichner Focus | Morrison Focus | Output to Build |
|---|---|---|---|
| Scalar field substrate | Ch. 3, Appendix B | state variables, model boundaries | 2D scalar grid with sampling, gradients, visualization |
| Clocks and proper time | Ch. 39, then Ch. 13 | model assumptions | stationary clocks accumulating local proper time |
| Gradient steering and pseudo-gravity | Ch. 7, 8, 13 | feedback loops, response curves | particle acceleration from a time-field gradient |
| Orbit comparison harness | Ch. 6, 7, 8, 13 | comparing model outputs | Newtonian orbit vs. toy-gradient orbit overlay |
| Matter density to potential field | Ch. 13; supplement outside Serway for Poisson/Laplacian numerics | source-response modeling | density layer feeding a potential/time field |
| Wave propagation in the time field | Ch. 15, 16, 18, 34 | dynamic systems and modes | wave equation on a 2D field with interference |
| Metric/geodesic-inspired motion | Ch. 39; external tensor/GR resources | model escalation | geodesic-like integrator and scalar-vs-metric comparison |
| Quantum-inspired experiments | Ch. 40, 41 | state evolution | simple wavefunction/probability/interference sandbox |
| Material and condensed-matter analogs | Ch. 43; external sources later | multiscale modeling | lattice/material-inspired field experiments |

## Study Method

For each chapter in the "Read Carefully" list:

1. Read the concepts and worked examples.
2. Do 3-5 problems that involve vectors, energy, fields, waves, relativity, or quantum behavior.
3. Convert one idea into a simulation diagnostic.
4. Write a short note with this structure:

```text
Real physics:
What does the textbook say should happen?

Toy model:
What simplified rule am I actually simulating?

Diagnostic:
What can I measure or visualize?

Break point:
Where does the toy model stop matching real physics?
```

This keeps the reading tied to code. The win condition is not finishing every problem. The win condition is turning hard physics ideas into working, testable simulation behavior.

## Suggested Reading Order

This order matches the first portfolio build arc.

1. Serway Ch. 3 and Appendix B as needed.
2. Morrison, begin straight-through reading.
3. Serway Ch. 7 and 8.
4. Serway Ch. 13.
5. Serway Ch. 6.
6. Serway Ch. 15, 16, and 18.
7. Serway Ch. 34.
8. Serway Ch. 39.
9. Serway Ch. 40 and 41.
10. Serway Ch. 43.

Skim Ch. 1, 2, 4, and 5 whenever the math or notation feels rusty. Pull in Ch. 9-11 and Ch. 14 only when a simulation demands them.

## Chapter-Level Success Criteria

Use these as exit checks. Move on when you can do the listed things well enough to code with them.

| Chapter | Move On When You Can... |
|---|---|
| 3 | decompose vectors, normalize directions, compute dot products, and explain gradients as directional change. |
| 6 | explain centripetal acceleration and set up a circular/orbital initial condition. |
| 7-8 | connect force, work, kinetic energy, potential energy, and conservative gradients. |
| 13 | explain inverse-square gravity, gravitational potential, and why orbital tests are a good diagnostic. |
| 15-16 | explain oscillation, wave speed, wavelength, frequency, and finite-difference wave updates conceptually. |
| 18 | explain superposition, constructive/destructive interference, and standing modes. |
| 34 | explain that waves carry energy and that field disturbances propagate at characteristic speeds. |
| 39 | distinguish coordinate time from proper time and compute simple time-dilation factors. |
| 40-41 | explain wave-particle duality, probability amplitudes, and why interference is not just classical particle motion. |
| 43 | explain why lattices, collective excitations, and material structure can produce effective behaviors. |

## Notes for Portfolio Framing

When discussing the curriculum in the portfolio, frame it like this:

> I did not use the physics textbook as a full course checklist. I used it as a targeted calibration source for simulations. Each selected chapter corresponds to a concrete simulation feature, diagnostic, or comparison target.

This makes the project look focused and honest. It shows that the learning is guided by engineering outputs, not vague completionism.

For Chapter 3 vector work, use `docs/chapter-3-vector-simulator-seeds.md` as the first concrete set of simulator candidates.

## Assumptions and Sources

Assumptions:

- You already know basic Newtonian mechanics conceptually.
- You want more mathematical fluency, especially around vectors, energy, fields, waves, relativity, and quantum ideas.
- The project goal is portfolio-grade simulation and modeling skill, not proving physics.
- Serway/Beichner is used as a targeted calibration source, not a full course.

Sources used:

- Local repo docs: `docs/portfolio-framing.md`
- Local repo docs: `docs/The Simulation-Physics Ladder  A Step-by-Step Build Plan for the Time Dilation Field Framework.md`
- Serway/Beichner fifth edition table of contents reference: https://www.tbooks.solutions/physics-for-scientists-and-engineers-serway-5th-edition/
