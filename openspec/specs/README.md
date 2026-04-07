# Spec Catalog

This file is the authoritative index of all OpenSpec specifications. Specs are organised **by domain** — one spec file per domain — following the OpenSpec convention of `openspec/specs/<domain>/spec.md`. Each domain spec is a single, coherent document that describes all behavior within that domain; changes to existing behavior produce `## MODIFIED Requirements` deltas against the appropriate domain spec.

> **Maintenance:** every change that adds, modifies, or removes requirements MUST update this file if the affected domain spec changes.

---

## Domains

Eight domains cover the full project surface. Each new requirement belongs to exactly one domain — use the [Decision Rules](#decision-rules) below to assign it.

| Domain | Spec | What belongs here |
|--------|------|------------------|
| [Infrastructure](#infrastructure) | [`infrastructure/spec.md`](infrastructure/spec.md) | Build system, developer tooling, devcontainer, task runners, R toolchain, spec catalog governance |
| [Data](#data) | [`data/spec.md`](data/spec.md) | Canonical data files, code-generation pipelines, game terminology glossary |
| [Game Core](#game-core) | [`game-core/spec.md`](game-core/spec.md) | Fundamental game-state types and initial setup structures |
| [Game Logic](#game-logic) | [`game-logic/spec.md`](game-logic/spec.md) | Rules: legal moves, move application, terminal detection, scoring |
| [Engine](#engine) | [`engine/spec.md`](engine/spec.md) | Game loop, play drivers, NDJSON logging, seed/setup plumbing |
| [TUI](#tui) | [`tui/spec.md`](tui/spec.md) | Terminal display rendering, keyboard input, game session launch, undo/redo history |
| [Agents](#agents) | [`agents/spec.md`](agents/spec.md) | Concrete decision-making strategies and the agent interface |
| [Analysis](#analysis) | [`analysis/spec.md`](analysis/spec.md) | R analysis scripts, plot outputs, statistical summaries of game data |

---

## Infrastructure

> Build, CI, developer environment, task automation, R toolchain, and spec catalog governance.

[`infrastructure/spec.md`](infrastructure/spec.md) covers: Meson build system, Catch2 via wrap, standard directory layout, devcontainer, single-source toolchain install script, Copilot agent setup workflow, README quick-start, clang-format/markdownlint mise tasks, mise as primary project entry point, R PPM binary install, and the spec catalog index requirements.

---

## Data

> Canonical data files that are the single source of truth; code-generation pipelines; shared game terminology.

[`data/spec.md`](data/spec.md) covers: `data/patches.yaml` YAML catalog, ASCII art shape encoding, R codegen producing committed `patches.hpp`, and the `docs/glossary.md` game terminology glossary.

---

## Game Core

> Fundamental data types that represent game state, plus the initial arrangement of game components. These are the **nouns** of the system.

[`game-core/spec.md`](game-core/spec.md) covers: `PlayerState` (128-bit bit-packed), `GameState` (combined player + shared state), `SimplifiedGameState` (next_player, first_to_finish), and `GameSetup` (initial patch circle arrangement, NDJSON serialisation, generated `game_setups.hpp`).

---

## Game Logic

> The **verbs** of the system: which moves are legal, how a move transforms state, when the game ends, and how the score is computed.

[`game-logic/spec.md`](game-logic/spec.md) covers: `Move` type (BuyPatch / Advance), `legal_moves`, `apply_move` (button cost, time cost, income, leather patches, 7×7 bonus), `is_terminal`, `score`, and `winner`.

---

## Engine

> The game loop and everything that makes a game run reproducibly end-to-end.

[`engine/spec.md`](engine/spec.md) covers: play driver executable (`--seed`, `--setup`, `--output`), NDJSON event logging (game-start, move, game-end events), and NDJSON format constraints.

---

## TUI

> The interactive terminal user interface for human play.

[`tui/spec.md`](tui/spec.md) covers: box-framed display rendering, patch circle with adaptive detail lines, wide/narrow layouts, color, resizable NDJSON log pane, scrolling event log, single-keypress input, RawMode guard, command legality enforcement, launch screen, game-result summary, and undo/redo history stack.

---

## Agents

> Concrete decision-making strategies.

[`agents/spec.md`](agents/spec.md) covers: random agent (uniform move selection, reproducibility via seed, standalone compilation).

---

## Analysis

> Offline analysis: R scripts, DuckDB queries, plots, and statistical tables produced from game logs or the patch catalog.

[`analysis/spec.md`](analysis/spec.md) covers: shape feature extraction (cells, perimeter, density), patch gain model (placement gain + projected income, normalised by time cost), time-position-dependent patch gain curves, summary CSV, and plots.

---

## Decision Rules

Apply these rules in order. The **first rule that matches** determines the domain. If still ambiguous after applying all rules, prefer the narrower domain.

1. **Infrastructure** — if the requirement is about the build system, CI pipeline, developer environment (devcontainer), task automation (mise), R toolchain setup, or spec catalog governance. These requirements have no game logic.
2. **Data** — if the requirement defines the schema or loading of a canonical data file, a code-generation pipeline that produces committed code from data files, or the shared terminology glossary.
3. **Game Core** — if the requirement defines a game-state *type* (struct, bitfield layout, encoding) or the initial arrangement of game components (setup, patch circle). Describes *what the state looks like*, not *how it changes*.
4. **Game Logic** — if the requirement defines *which moves are legal*, *how a move transforms state*, *when the game ends*, or *how the score is computed*.
5. **TUI** — if the requirement is about interactive terminal rendering, keyboard input handling, the game session launch screen, or undo/redo history. These requirements are about the user-facing UI layer.
6. **Engine** — if the requirement is about running a game end-to-end non-interactively: the play loop, a driver executable, NDJSON event logging, or seed/setup reproducibility plumbing.
7. **Agents** — if the requirement defines a concrete decision strategy or the interface an agent must implement. Tiebreaker: if a requirement could be Engine or Agents, ask whether it is reusable across different game loops (Agents) or tied to one specific driver (Engine).
8. **Analysis** — if the requirement is about offline analysis of game data: R scripts, plots, statistical tables, or DuckDB queries.

---

## Naming Convention

- Domain spec folders use **kebab-case** (lowercase words separated by hyphens).
- The folder name describes the **domain**, not an implementation detail: `game-logic` not `move-generator`.
- Prefer the **noun phrase** that names the domain: `game-core`, `game-logic`, `engine`, `tui`.
- Avoid encoding hierarchy in the name: `game-core` not `patchwork-game-core`.
