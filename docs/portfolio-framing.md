# Portfolio Framing

## Project Thesis

This project is a C++23 high-performance physics sandbox for exploring complex physics-inspired systems through simulation. It is not a scientific paper, a proof, or a claim about new physics. The goal is to build rigorous, inspectable models that borrow patterns from relativity, quantum mechanics, field theory, and numerical simulation, then use those models to discover interesting behaviors, visual patterns, and engineering challenges.

The core idea is simple:

> Build sandbag models that are playful enough to explore, grounded enough to compare against real physics, and engineered well enough to stand as a serious portfolio project.

## What This Project Is

- A simulation-first playground for physics-inspired models.
- A portfolio demonstration of complex systems engineering in modern C++.
- A way to learn and apply concepts from relativity, quantum mechanics, fields, waves, metrics, numerical methods, and high-performance computing.
- A space for finding useful computational patterns, not proving physical theories.

## What This Project Is Not

- Not a replacement for established physics.
- Not a unified field theory.
- Not a formal research report.
- Not a claim that the toy models describe reality exactly.

The project should stay honest: established physics, toy simulation hypotheses, and speculative extensions should remain clearly separated.

## Portfolio Pillars

### 1. Physics-Inspired Simulation

The project uses real physics concepts as inspiration and calibration targets:

- scalar and vector fields
- proper time and coordinate time
- time dilation
- gradient-driven motion
- wave propagation
- particle-field coupling
- weak-field relativity approximations
- metric and geodesic-inspired motion
- quantum-inspired state evolution and interference patterns

The simulator does not need to be fully faithful to general relativity or quantum field theory to be valuable. Its value comes from building models, comparing their behavior to known physics, and learning where they break.

### 2. Engineering Rigor

The codebase should demonstrate professional systems engineering:

- C++23
- RAII
- smart pointers and explicit ownership
- deterministic simulation stepping
- clean module boundaries
- strong typing for units and simulation parameters where useful
- testable physics kernels
- reproducible experiments
- benchmarkable performance paths
- clear separation between simulation, visualization, and UI

The goal is not just to make a cool visual toy. The goal is to show that complex simulation work can be built cleanly, maintained, profiled, and extended.

### 3. HPC and GPU Path

The project should start with correct, readable CPU implementations, then evolve toward performance:

- cache-friendly grid layouts
- structure-of-arrays data design where appropriate
- SIMD-friendly math kernels
- multithreaded update loops
- job systems or task partitioning
- GPU compute paths for field updates, particles, and visualization
- profiling-driven optimization

Performance work should be motivated by measured bottlenecks. The portfolio value comes from showing the path from simple model to optimized system.

## Physics Stance

The project should keep physics real without becoming stiff.

For each model, ask:

1. What real physics pattern inspired this?
2. What simplified rule are we actually simulating?
3. What behavior should we expect?
4. Where does the toy model diverge from established physics?
5. Does the divergence reveal an interesting pattern, limitation, or next experiment?

This keeps the project playful without letting it drift into unsupported claims.

## Recommended Framing

Short version:

> A C++23 high-performance physics sandbox for experimenting with relativity-inspired and quantum-inspired field models.

Portfolio version:

> This project explores complex physics-inspired simulation using modern C++23, rigorous ownership and architecture patterns, and a performance path toward HPC and GPU acceleration. It uses concepts from relativity, quantum mechanics, fields, waves, and numerical methods as modeling inspiration, while clearly treating the results as sandbox simulations rather than scientific claims.

More personal version:

> I am using this project to learn difficult physics by building it. The models are intentionally sandbag models: simplified, inspectable, and breakable. The point is to explore patterns, compare them against real physics where possible, and demonstrate that I can turn abstract physical ideas into robust, performant simulation code.

## Suggested Build Direction

Use the targeted reading path in `docs/selective-physics-curriculum.md` alongside this build direction. The curriculum maps Serway/Beichner chapters and *The Art of Modeling Dynamic Systems* to the specific simulations below.

1. Build a clean scalar field substrate.
2. Add clocks and proper-time accumulation.
3. Add particles and gradient-driven motion.
4. Compare against Newtonian inverse-square behavior.
5. Add matter-density-to-field experiments.
6. Add wave propagation and dynamic fields.
7. Add metric/geodesic-inspired motion.
8. Add quantum-inspired state and interference experiments.
9. Introduce benchmarking and profiling.
10. Move selected kernels to optimized CPU and GPU paths.

Each step should produce something visible, measurable, and explainable.

## North Star

The strongest version of this project is not one that claims to rewrite physics. It is one that shows:

- curiosity about hard ideas
- discipline in separating models from reality
- skill in modern C++ architecture
- ability to build and test numerical simulations
- ability to optimize complex systems
- taste in turning abstract physics into interactive, inspectable software

That is the portfolio story.
