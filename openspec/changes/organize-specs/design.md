## Context

The `openspec/specs/` directory had grown to 19 capability-level spec files (`build-system`, `game-state`, `move-generation`, `tui-display`, etc.) with no domain grouping. The [OpenSpec documentation](https://github.com/Fission-AI/OpenSpec/blob/main/docs/concepts.md#specs) is explicit: specs should be organised by domain (`auth/spec.md`, `payments/spec.md`), not by individual capability. Domain-level specs serve as coherent contracts — a single document describes all behavior within a domain, and changes produce targeted deltas against it. Capability-level specs fragment related requirements and make `MODIFIED Requirements` deltas harder to issue because an author must find the right small file rather than the right domain.

Without domain organisation, contributors and AI agents:
- Scatter related requirements across many small files
- Create redundant specs that partially overlap with existing ones
- Have no single reference for the current full capability surface

The fix is to merge all 19 capability-level specs into 7 domain-level spec files, add a `README.md` catalog, and update the active `r-package-structure` change to target the new domain names.

## Goals / Non-Goals

**Goals:**
- Restructure `openspec/specs/` to one `spec.md` per domain following the OpenSpec-documented pattern.
- Define a fixed domain taxonomy aligned with roadmap phases.
- Add TUI as a distinct domain (the four TUI specs are a coherent interactive UI layer).
- State unambiguous decision rules so any new requirement can be assigned to exactly one domain.
- Document the naming convention.
- Update active change deltas to target domain names.

**Non-Goals:**
- Two-level nesting (`openspec/specs/game-core/game-state/`). The OpenSpec `archive` CLI maps `changes/<change>/specs/<entry>/spec.md` one-to-one to `openspec/specs/<entry>/spec.md` — the entry is just the domain folder name.
- Automating spec discovery or generating the index programmatically.

## Decisions

**Decision: Domain-level spec files**
One `spec.md` per domain, not per capability. Requirements within a domain are collected in a flat `## Requirements` section. This is the OpenSpec-documented pattern and enables `MODIFIED Requirements` deltas to target a single file per domain.

**Decision: Flat `## Requirements` section within each domain spec**
The OpenSpec CLI (`specs-apply.js`) parses requirements from a single `## Requirements` section, stopped at the next `## ` header. Sub-grouping within Requirements via additional `##` headers would truncate the section or cause other requirements to be invisible to the archive command. Requirements remain flat; their names provide enough context (e.g., "Project builds with Meson" vs "Devcontainer provides a complete build environment").

**Decision: Eight domains (adding TUI)**
The original seven-domain taxonomy omitted TUI. The four TUI specs (`tui-display`, `tui-input`, `tui-launch`, `tui-undo-redo`) form a coherent interactive UI layer matching OpenSpec's `ui/` domain example. TUI is added as the sixth domain (between Engine and Agents).

| Domain | Spec file | What belongs here |
|--------|-----------|------------------|
| **Infrastructure** | `infrastructure/spec.md` | Build system, developer tooling, devcontainer, task runners, R toolchain, spec catalog |
| **Data** | `data/spec.md` | Canonical data files, code-generation pipelines, glossary |
| **Game Core** | `game-core/spec.md` | Game-state types, initial setup structures |
| **Game Logic** | `game-logic/spec.md` | Legal moves, move application, terminal detection, scoring |
| **Engine** | `engine/spec.md` | Play drivers, NDJSON logging, seed/setup plumbing |
| **TUI** | `tui/spec.md` | Terminal display, keyboard input, launch screen, undo/redo history |
| **Agents** | `agents/spec.md` | Concrete decision-making strategies |
| **Analysis** | *(future)* | Offline analysis scripts, plots, statistical summaries |

**Decision rules (one domain per requirement)**
1. Build, CI, developer environment, task automation, R toolchain, spec catalog governance → **Infrastructure**
2. Canonical data files, code-generation pipelines, glossary → **Data**
3. Game-state *type* (struct, encoding) or initial arrangement of game components → **Game Core**
4. What is legal, what happens when a move is applied, game end, scoring → **Game Logic**
5. Interactive terminal rendering, keyboard input, launch screen, undo/redo → **TUI**
6. Running a game end-to-end non-interactively (loop, driver, logging, reproducibility) → **Engine**
7. Concrete decision strategy or agent interface → **Agents**
8. Offline analysis, plots, statistical summaries → **Analysis**

Tiebreaker: if a requirement fits two domains, apply the higher-numbered rule that fits (more specific wins). If still ambiguous, prefer the narrower domain.

**Decision: Update r-package-structure deltas to target domain names**
The active `r-package-structure` change had deltas using `## MODIFIED Requirements` targeting `game-setup` and `patch-catalog` — but those requirements did not exist in those specs. The deltas have been updated to `## ADDED Requirements` targeting `game-core` and `data` respectively, and the `r-package-infra` delta now targets `infrastructure`.

## Risks / Trade-offs

- [Risk] README can drift out of sync as new requirements are added. → Mitigation: each future change's `tasks.md` MUST include a step to update the catalog.
- [Risk] Domain boundaries are sometimes ambiguous. → Mitigation: numbered decision rules with a tiebreaker; edge cases are cheap to re-classify since only the domain spec and README change.
