## Context

The `openspec/specs/` directory holds the canonical, living specification of every system capability. Specs accumulate as phases are completed — each archived change promotes its specs into `openspec/specs/`. With seven specs already present and more arriving each phase, the flat directory is increasingly hard to navigate and gives AI agents no signal about domain groupings or naming conventions for new specs.

## Goals / Non-Goals

**Goals:**
- Add a single `openspec/specs/README.md` that serves as the authoritative index and taxonomy for all specs.
- Group every existing spec into a named domain.
- Define a naming convention and domain list that future changes can follow.

**Non-Goals:**
- Moving or renaming existing spec folders (file paths are stable identifiers; do not break them).
- Automating spec discovery or generating the index programmatically.
- Enforcing conventions via tooling (convention-by-documentation is sufficient at this scale).

## Decisions

**Decision: README.md in `openspec/specs/`**
A plain Markdown file is the lightest possible solution. It is human-readable in editors and on GitHub, AI-readable as plain text, and requires no tooling. Alternatives (a YAML manifest, a generated HTML page) add complexity with no benefit at the current scale.

**Decision: Domain taxonomy based on roadmap phases**
The roadmap already partitions the project into coherent phases (Infrastructure, Data, Game Core, Game Logic, Engine, Agents, Analysis). Using the same language in the spec catalog makes it easy to trace from roadmap phase → change → specs. Alternative (alphabetical or chronological order) provides no semantic grouping.

**Decision: Flat spec folders remain unchanged**
Physically grouping specs into subfolders (e.g. `specs/core/game-state/`) would break existing `openspec status` and archive tooling. The README provides logical grouping without touching file paths.

## Risks / Trade-offs

- [Risk] README can drift out of sync as new specs are added. → Mitigation: add a task step to each future change's `tasks.md` reminding the implementer to update the index.
- [Risk] Domain boundaries are sometimes ambiguous (e.g. `game-setup` could be Data or Game Core). → Mitigation: the README includes a short rationale for each domain; edge cases can be re-classified cheaply.
