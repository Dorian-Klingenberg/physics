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
