# Spec Catalog

This file is the authoritative index of all OpenSpec specifications. It organises specs by domain and defines the rules for naming and placing future specs.

> **Maintenance:** every change that adds or removes a spec MUST update this file.

---

## Domains

Seven domains cover the full project surface. Each new spec belongs to exactly one domain — use the [Decision Rules](#decision-rules) below to assign it.

| Domain | What belongs here |
|--------|------------------|
| [Infrastructure](#infrastructure) | Build system, developer tooling, devcontainer, task runners |
| [Data](#data) | Canonical data files, code-generation pipelines, patch catalog |
| [Game Core](#game-core) | Fundamental game-state types and initial setup structures |
| [Game Logic](#game-logic) | Rules: legal moves, move application, terminal detection, scoring |
| [Engine](#engine) | Game loop, play drivers, NDJSON logging, seed/setup plumbing |
| [Agents](#agents) | Any concrete decision-making strategy or agent interface |
| [Analysis](#analysis) | R analysis scripts, plot outputs, statistical summaries of game data |

---

## Infrastructure

> Build, CI, developer environment, and task automation.

| Spec | Description |
|------|-------------|
| [`build-system`](build-system/spec.md) | Meson build system setup — `meson setup build && ninja -C build` produces a working test binary |
| [`devcontainer`](devcontainer/spec.md) | `.devcontainer/devcontainer.json` provides a complete build environment with no manual setup |
| [`mise-tasks`](mise-tasks/spec.md) | Every useful project action has a `mise run <task>` entry in `.mise.toml` |

---

## Data

> Canonical data files that are the single source of truth; code-generation pipelines that produce committed headers from those files.

| Spec | Description |
|------|-------------|
| [`patch-catalog`](patch-catalog/spec.md) | `data/patches.yaml` is the sole source of patch definitions; R codegen produces the committed C++ header |

---

## Game Core

> Fundamental data types that represent game state, plus the initial arrangement/setup of the game. These are the nouns of the system.

| Spec | Description |
|------|-------------|
| [`game-state`](game-state/spec.md) | `PlayerState` (128-bit) and `GameState` packed shared state types |
| [`simplified-game-state`](simplified-game-state/spec.md) | `SimplifiedPlayerState` (32-bit) and `SimplifiedGameState` without quilt board |
| [`game-setup`](game-setup/spec.md) | `GameSetup` holds the initial patch circle arrangement and seeded RNG |

### In progress (`simplified-rules`)

| Spec | Description |
|------|-------------|
| `simplified-game-state` *(delta)* | Adds `next_player` field to `SimplifiedGameState` |

---

## Game Logic

> The verbs of the system: what moves are legal, what happens when a move is applied, when the game ends, and how the score is computed.

*(No archived specs yet — arriving with `simplified-rules`)*

### In progress (`simplified-rules`)

| Spec | Description |
|------|-------------|
| `move-generation` | `Move` type and `legal_moves(state)` — all legal actions in the simplified ruleset |
| `move-application` | `apply_move(state, move)` — state transitions for BuyPatch and Advance |
| `terminal-and-scoring` | `is_terminal`, `score`, leather-patch awards, 7×7 bonus |

---

## Engine

> The game loop and everything that makes a game run reproducibly end-to-end: play drivers, NDJSON game logging, seed and setup plumbing.

*(No archived specs yet — arriving with `simplified-rules`)*

### In progress (`simplified-rules`)

| Spec | Description |
|------|-------------|
| `play-driver` | Executable that runs a full game between two agents and writes a NDJSON log |
| `game-logger` | NDJSON event logging — game-start, move, game-end events |

---

## Agents

> Concrete decision-making strategies. Each agent implements a shared interface and can be plugged into the play driver.

*(No archived specs yet — arriving with `simplified-rules`)*

### In progress (`simplified-rules`)

| Spec | Description |
|------|-------------|
| `random-agent` | Selects a legal move uniformly at random |

---

## Analysis

> Offline analysis: R scripts, DuckDB queries, plots, and statistical tables produced from game logs or the patch catalog.

*(No specs yet — arriving with Tile Analysis phase)*

---

## Decision Rules

Apply these rules in order. The **first rule that matches** determines the domain. If a spec fits two rules equally, prefer the rule with the **lower number** (broader scope first, then narrow). If still ambiguous after applying all rules, prefer the narrower domain.

1. **Infrastructure** — if the spec is about the build system, CI pipeline, developer environment (devcontainer), or task automation (mise). These specs have no game logic.
2. **Data** — if the spec defines the schema or loading of a canonical data file, or a code-generation pipeline that produces committed code from data files.
3. **Game Core** — if the spec defines a game-state *type* (a struct, bitfield layout, or encoding) or the initial arrangement of game components (setup, patch circle). These specs describe *what the state looks like*, not *how it changes*.
4. **Game Logic** — if the spec defines *which moves are legal*, *how a move transforms state*, *when the game ends*, or *how the score is computed*. These specs describe the rules of the game.
5. **Engine** — if the spec is about running a game end-to-end: the play loop, a driver executable, NDJSON event logging, or seed/setup reproducibility plumbing. These specs orchestrate Game Logic and Agents.
6. **Agents** — if the spec defines a concrete decision strategy or the interface an agent must implement. Tiebreaker: if a spec could be Engine or Agents, ask whether the spec is reusable across different game loops (Agents) or tied to one specific driver (Engine).
7. **Analysis** — if the spec is about offline analysis of game data: R scripts, plots, statistical tables, or DuckDB queries. These specs produce human-readable artefacts, not runtime code.

---

## Naming Convention

- Spec folder names use **kebab-case** (lowercase words separated by hyphens).
- The name describes the **capability**, not the implementation type: `move-generation` not `move-generator` or `legal-moves-function`.
- Prefer the **noun phrase** that names the concept: `game-state`, `patch-catalog`, `random-agent`.
- If a spec is a refinement of an existing capability, prefix with the parent: `simplified-game-state` refines `game-state`.
- Avoid encoding the domain in the name: `build-system` not `infrastructure-build-system`.
