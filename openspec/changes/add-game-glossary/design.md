## Context

Game concepts such as "patch," "time track," "button income," and "quilt board" are used throughout source code, YAML data files, OpenSpec specs, and AI agent prompts — but without a single authoritative reference. Contributors (human and AI) must infer term meanings from context. As the project grows (more specs, agents, analysis scripts), inconsistent terminology creates confusion and drift.

The single-source-of-truth principle already applies to patch data (`data/patches.yaml`); this change extends it to domain vocabulary.

## Goals / Non-Goals

**Goals:**
- A canonical YAML glossary (`data/glossary.yaml`) that is the single source of truth for game and engine terminology
- A human-readable Markdown companion (`docs/glossary.md`) — either generated from the YAML or kept in sync
- Integration into OpenSpec context so AI agents automatically consult the glossary
- Links from `README.md` and relevant agent prompts

**Non-Goals:**
- Automated code generation from glossary terms (no C++ header, no enum from glossary)
- Enforcing term usage in code via linting (future concern)
- Translating the glossary to other languages
- Covering implementation-internal terms (e.g., `PlayerState` bit layout details) — only game/engine concepts

## Decisions

### Decision: YAML as machine-readable source, Markdown as human-readable view

**Choice:** `data/glossary.yaml` is the canonical source; `docs/glossary.md` is a human-friendly rendering maintained alongside it.

**Rationale:** YAML is already the project's data format (patches, setups). Keeping the glossary in YAML lets future tooling (codegen, validation scripts) consume it without parsing Markdown. The Markdown file gives humans a readable reference without needing to understand YAML structure. Maintaining both files manually is low overhead since the glossary changes infrequently.

**Alternative considered:** Markdown-only glossary. Rejected because it is harder to parse programmatically and breaks the single-source-of-truth convention.

**Alternative considered:** Auto-generate Markdown from YAML via an R script. Deferred — adds tooling complexity without immediate benefit; the glossary is small enough to maintain manually.

### Decision: YAML schema — flat list with `term`, `definition`, `aliases`, `see_also`, `category`

**Choice:** Each entry has:
- `term` (string, required): canonical name
- `definition` (string, required): one or two sentence explanation
- `aliases` (list of strings, optional): alternative names used in code or rules
- `see_also` (list of strings, optional): related terms by canonical name
- `category` (string, required): grouping — one of `game-rules`, `engine`, `data`

**Rationale:** Flat schema is easy to author and extend. Categories allow future tooling to filter by domain. Aliases capture the fact that "tile" and "patch" are used interchangeably. `see_also` enables navigation without hardcoded cross-links.

### Decision: Reference glossary from OpenSpec config context

**Choice:** Add a line to `openspec/config.yaml`'s `context` block pointing AI agents to `data/glossary.yaml` and `docs/glossary.md`.

**Rationale:** The OpenSpec context field is already read by AI agents before proposing or implementing changes. Adding the reference there ensures every future agent invocation is aware of the glossary without modifying individual prompt files.

### Decision: Update `.github/prompts/` shared agent prompt files

**Choice:** Add a brief mention of the glossary in the relevant prompt files (at minimum the apply prompt, and any shared context file if present).

**Rationale:** Belt-and-suspenders: some agent invocations may bypass OpenSpec context but still read prompt files.

## Risks / Trade-offs

- **Glossary drift** → Mitigation: Define a clear authoring convention (human updates glossary when new terms appear in roadmap/specs). The OpenSpec spec for `game-glossary` will include a requirement that new terms be added alongside the changes that introduce them.
- **Markdown and YAML getting out of sync** → Mitigation: Keep entries in the same order in both files; note in both files that they should be kept in sync. Future: R script to validate or regenerate.
- **Over-specification of initial terms** → Mitigation: Start with a focused list of terms that already appear in existing specs and code; explicitly mark the glossary as "initial list, to be expanded."

## Open Questions

- Should an R validation script check that all terms in `data/glossary.yaml` that appear in spec files are consistently capitalised? (Deferred to a future change.)
- Should `docs/` directory be created, or should `glossary.md` live at the root or alongside `README.md`? (Decision: create `docs/glossary.md` to keep documentation organised and extensible.)
