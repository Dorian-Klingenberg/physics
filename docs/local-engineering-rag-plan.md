# Local Engineering RAG Plan

## Purpose

This is a future build plan, not an active implementation task.

The goal is to give smaller coding models fast local access to trusted
engineering guidance so they do not rely only on model memory or web research
for C++23, RAII, clean architecture, design patterns, and project-specific
coding standards.

The intended result is a local engineering memory:

- queryable from agents and scripts
- grounded in trusted sources
- compact enough to be token-efficient
- fast enough to use before ordinary C++ work
- explicit about when advice is mandatory versus advisory

## Architectural Recommendation

Build this as a layered engineering system, not as "RAG first".

The recommended architecture is:

```text
1. Enforceable tooling
   clang-format, clang-tidy, compiler warnings, tests

2. Tiny always-on project rules
   AGENTS.md, .github/copilot-instructions.md, docs/engineering-standards.md

3. On-demand local retrieval
   standards docs, cppreference, C++ Core Guidelines, Win32/D3D12,
   design-pattern notes, clean-code notes

4. Assistant workflow
   retrieve only when relevant, cite sources, then code

5. Evaluation loop
   fixed questions plus lint/build/test outcomes
```

The big design decision is that RAG should be the memory layer, not the
enforcement layer. Static tooling should catch what can be checked
deterministically. RAG should explain, guide, and retrieve source-backed
judgment when the answer depends on context.

## What This Should And Should Not Do

This system should improve correctness, consistency, and speed of lookup. It
should not train or fine-tune a model.

Use it to retrieve concise guidance at coding time:

- what C++23 feature or standard-library type applies
- how RAII ownership should be represented
- what the C++ Core Guidelines say about ownership, lifetime, and interfaces
- which design pattern fits a real variation point
- what this project's own architecture rules prefer

Do not use it to make agents blindly apply patterns everywhere. A retrieved
design pattern is a candidate, not an instruction. The default should remain
simple code until a real design pressure appears.

## Layer 1: Enforceable Tooling

Start here before building any retrieval system.

This layer costs no prompt tokens and catches real C++ problems mechanically.
It should become the first authority for formatting, modern C++ checks,
readability checks, and many C++ Core Guidelines-adjacent rules.

Recommended files and tools:

- `.clang-format`
- `.clang-tidy`
- CMake support for `compile_commands.json`
- strict but practical compiler warnings
- focused unit tests for simulation kernels
- optional later tools such as Include-What-You-Use

Useful clang-tidy check groups for this project:

- `cppcoreguidelines-*`
- `modernize-*`
- `bugprone-*`
- `performance-*`
- `readability-*`
- `clang-analyzer-*`

This layer should enforce what can be enforced:

- obvious lifetime and ownership mistakes
- avoidable raw pointer patterns
- modernization opportunities
- bug-prone constructs
- readability issues that do not depend on taste alone
- performance issues that tooling can reliably detect

Do not ask a language model to remember rules that a tool can check faster and
more consistently.

## Layer 2: Tiny Always-On Rules

Always-on instructions should be short because they are repeatedly injected
into coding workflows.

Use this layer for project invariants:

```text
Use C++23.
Prefer RAII and explicit ownership.
Keep simulation logic independent from ImGui and DirectX 12.
Keep renderer code separate from physics kernels.
Use CPU reference paths before GPU optimization.
Use design patterns only when they solve a real variation point.
Consider clang-tidy/build/test results before finalizing C++ changes.
```

Do not put long essays, copied blog posts, or full design-pattern descriptions
into always-on files. Those belong in the retrieval layer.

Good always-on files:

- `AGENTS.md`
- `.github/copilot-instructions.md`
- path-specific `.github/instructions/*.instructions.md` files
- a short pointer to `docs/engineering-standards.md`

The always-on layer should tell the agent what posture to take. It should not
try to be the entire knowledge base.

## Recommended Stack

Prefer established RAG tools over a homegrown retrieval system.

- **Parser/converter:** Docling for PDFs and rich documents; direct Markdown and
  HTML loaders for clean source material.
- **RAG framework:** LlamaIndex or Haystack.
- **Local vector store:** Qdrant. Chroma is an acceptable simpler fallback.
- **Retrieval mode:** hybrid retrieval, combining semantic vector search with
  keyword/BM25-style search and metadata filters.
- **Evaluation:** a small Ragas-style question set that checks retrieval and
  answer quality before the system becomes trusted.

Avoid building custom embedding storage, custom chunking machinery, or custom
reranking until an existing tool is clearly insufficient.

## Layer 3: On-Demand Local Retrieval

This is the actual RAG layer.

Use retrieval when the model needs source-backed context:

- exact C++23 or standard-library behavior
- C++ Core Guidelines rationale
- Win32, COM, DirectX 12, or platform specifics
- project architecture precedents
- whether a design pattern fits a real situation
- clean-code tradeoffs that need judgment

Do not retrieve for every tiny edit. For ordinary mechanical changes, local
code context plus tooling is enough.

The retrieval flow should be narrow:

```text
coding question
  -> classify topic
  -> apply metadata filters
  -> retrieve 3 to 6 compact chunks
  -> prefer house rules and hard standards
  -> use best-practice material only when architecture/design judgment matters
  -> answer or code with citations when retrieved sources matter
```

Example:

```text
Task: Add renderer-selection behavior.

Retrieve:
1. project renderer/simulator architecture card
2. Strategy pattern card
3. Factory pattern card, if construction policy is part of the question
4. C++ Core Guidelines note on ownership/interface design
5. existing project example, if available

Decision:
Use Strategy only if multiple renderer implementations are real now or clearly
planned. Keep direct concrete code if there is no stable variation point yet.
```

## Source Policy

Treat source choice as part of the architecture.

### Hard Standards Corpus

Use these as authoritative or near-authoritative references:

- cppreference for C++ language and standard-library lookup
- C++ Core Guidelines for ownership, lifetime, interface, and RAII guidance
- Microsoft C++ / Win32 / DirectX 12 documentation where platform behavior
  matters
- project-local architecture documents in `docs/`

When these conflict with generic clean-code advice, prefer the hard standards
and the needs of this C++/simulation codebase.

### Best-Practices Corpus

Use these as advisory references:

- public design-pattern references
- public SOLID and clean-architecture articles
- selected Uncle Bob / Clean Coder blog posts
- other engineering essays that are intentionally chosen, not bulk-scraped

The system may retrieve these sources to inform a decision, but it should not
treat them as rules that override local architecture, performance constraints,
or C++ ownership correctness.

### House Rules Corpus

Create a compact project-authored standards document before indexing large
best-practices material. This should become the first retrieval target for
ordinary coding questions.

Suggested file when the system is built:

```text
docs/engineering-standards.md
```

It should summarize the standards this project actually wants:

- Prefer C++23, RAII, smart pointers, and explicit ownership.
- Make invalid states hard to represent.
- Keep simulation logic independent from ImGui and DirectX 12.
- Keep rendering and simulator state behind clear module boundaries.
- Use CPU reference paths before GPU optimization.
- Use design patterns only when they clarify a real variation point.
- Prefer Strategy for interchangeable behavior, Adapter for integration
  boundaries, Factory for nontrivial construction policy, and Observer/Event
  patterns only when direct ownership would create coupling.
- Do not introduce a pattern just to name a pattern.
- Prefer boring, readable functions until duplication or variation proves an
  abstraction is useful.
- Document performance-driven departures from clean-code defaults.

This house-rules layer should be short enough to retrieve in full.

## Best-Practice Cards

Do not make raw blog posts or full pattern articles the first thing models see.
Create compact project-authored cards and link them back to sources.

Cards should be short enough to retrieve whole. They should encode project
judgment, not merely summarize the internet.

Example card shape:

```text
Pattern: Strategy

Use when:
- behavior varies independently from the object using it
- multiple implementations are real or clearly planned
- selection may happen at runtime, configuration time, or module assembly time

Avoid when:
- there is only one implementation
- a simple function, value type, or direct call is clearer
- the abstraction exists only so the code can name a pattern

C++ note:
- prefer value semantics or function injection where practical
- use interfaces when lifetime and substitutability are explicit
- document ownership at the boundary

Sources:
- project architecture docs
- design-pattern reference
- C++ Core Guidelines interface/ownership guidance
```

Likely cards:

- RAII
- explicit ownership
- `std::unique_ptr` versus `std::shared_ptr`
- COM pointer ownership
- Strategy
- Factory
- Adapter
- Observer/Event
- Dependency Inversion
- Single Responsibility
- simulation/render/UI separation
- CPU reference path before GPU optimization
- when performance code may violate generic clean-code preferences

These cards become the bridge between broad best-practice writing and this
specific codebase.

## Licensing And Local Use

Keep provenance for every source.

For Wikipedia-derived material, preserve attribution and license metadata. For
blog material, especially posts that are public to read but not explicitly
licensed for redistribution, store only what is appropriate for local personal
retrieval and avoid publishing copied article text in this repository.

Prefer indexing URLs, titles, dates, local summaries, and short excerpts rather
than copying entire copyrighted articles into committed files.

## Metadata To Store

Every chunk should carry enough metadata for filtering and citations:

```text
source_id
source_title
source_url_or_path
license
corpus_kind        hard-standard | best-practice | house-rule | project-doc
topic_tags         cpp23, raii, ownership, d3d12, pattern-strategy, etc.
project_scope      physics, general-cpp, win32, directx, architecture
content_hash
indexed_at
page_or_section
```

For local project docs, include the repository path. For PDFs, include page or
section information when available.

## Retrieval Rules

The system should retrieve in this order:

1. House rules and project-local architecture docs.
2. Hard standards matching the language, platform, or API question.
3. Best-practices/design-pattern material only when the question is about
   structure, maintainability, boundaries, or naming.
4. Broader web research only when local sources are missing or stale.

For coding agents, the answer should include source references when the result
depends on a retrieved standard or external best-practice source.

## Layer 4: Assistant Workflow

Agents should use the system as a workflow, not as a passive pile of documents.

For normal C++ implementation:

```text
1. Read the local code around the requested change.
2. Check always-on project rules.
3. Retrieve only if the task touches standards, ownership, architecture,
   platform APIs, or design-pattern decisions.
4. Implement the smallest code change that fits the existing architecture.
5. Run or recommend the relevant build, tests, and clang-tidy checks.
6. Report which retrieved guidance affected the decision, if any.
```

For architecture questions:

```text
1. Retrieve house rules and project docs first.
2. Retrieve hard standards if C++ ownership/API behavior matters.
3. Retrieve best-practice cards only after the local design pressure is clear.
4. Compare options against this project's actual constraints.
5. Prefer boring direct code until duplication, lifetime, or variation justifies
   an abstraction.
```

For review:

```text
1. Use static-analysis output when available.
2. Retrieve standards for disputed or subtle issues.
3. Prioritize correctness, ownership, lifetime, API misuse, and missing tests.
4. Treat pattern advice as secondary unless the current design is already
   creating coupling or duplication.
```

## Token And Speed Strategy

RAG is not automatically token-efficient. It becomes efficient only when
retrieval is narrow.

Use:

- small top-k values by default, such as 3 to 6 chunks
- metadata filters before semantic search when the topic is known
- compact chunks, roughly 400 to 800 tokens, with overlap only where needed
- cached embeddings
- reranking only when retrieval quality is noisy

The intended efficiency win is fewer web searches, fewer correction loops, and
less repeated prompt text. Do not stuff large articles or whole chapters into
ordinary prompts.

## Layer 5: Evaluation Loop

The system should not become trusted just because it exists.

Evaluate three things:

1. **Retrieval quality:** did the right source or card appear?
2. **Answer faithfulness:** did the model follow the retrieved material?
3. **Engineering outcome:** did the resulting code build, lint, test, and fit
   the project architecture?

For code generation tasks, static tooling and tests are part of the evaluation
loop. A retrieved quote about RAII is not enough if the code still leaks
ownership or hides lifetime.

## Evaluation Set

Before trusting the system, create a small fixed test set.

Example questions:

- How should a DirectX 12 COM resource be owned in C++?
- When should a simulator depend on an interface instead of a concrete type?
- Should a renderer-selection system use Strategy, Factory, both, or neither?
- What does RAII require for a type that owns a GPU resource?
- When is `std::unique_ptr` preferred over `std::shared_ptr`?
- How should this project separate simulation state from ImGui controls?
- What clean-code advice should be ignored in a hot numerical kernel?

Track whether retrieval found the right source, whether the answer stayed
faithful to that source, and whether the answer avoided over-engineering.

## Build Later Checklist

When ready to build this:

1. Create a `rag/` or `tools/rag/` directory.
2. Add a source manifest modeled after the Granny's House Trials `books`
   manifest, but adapted to this project.
3. Write `docs/engineering-standards.md` as the house-rules corpus.
4. Ingest project docs and hard-standard sources first.
5. Add best-practices sources only after the hard-standard path works.
6. Store indexes outside normal source control unless they are intentionally
   small and portable.
7. Add generated agent instructions that tell future agents how to query the
   local corpus.
8. Add the fixed evaluation questions.

Do not start by bulk-copying the old Granny index. The useful inheritance is
the manifest/provenance discipline, not the specific SQLite FTS implementation.

## Recommended Build Order

When this becomes active work, build in this order:

1. Add `.clang-format`, `.clang-tidy`, warning policy, and CMake support for
   `compile_commands.json`.
2. Write `docs/engineering-standards.md` as the compact house-rules document.
3. Keep `AGENTS.md` and `.github/copilot-instructions.md` short, with links to
   deeper standards docs.
4. Build a local retrieval CLI, such as `tools/rag/query.py` or
   `tools/rag/query.ps1`.
5. Ingest project docs, cppreference, C++ Core Guidelines, and Microsoft
   Win32/DirectX material.
6. Create best-practice cards for design patterns and clean-code principles.
7. Add selected blog/design-pattern sources only as provenance-backed advisory
   material.
8. Add the evaluation question set.
9. Only then tune embeddings, chunk sizes, reranking, or editor integration.

This order keeps the useful parts available early while avoiding a large RAG
detour before the project has enforceable engineering guardrails.
