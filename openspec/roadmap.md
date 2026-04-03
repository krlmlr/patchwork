# Patchwork Engine — Roadmap

Uwe Rosenberg's Patchwork as a case study for game engine development, modern C++, and AI techniques from scratch to reinforcement learning.

## Principles

- **AI owns code, human owns design.** Each change is proposed and reviewed before implementation.
- **Single source of truth.** Canonical data (patch catalog, game setups) lives in data files; code is generated and committed.
- **Human-reviewable state.** Dense representations are used for performance, but readable abstractions sit on top.
- **Tests from day one.** No change ships without tests.
- **R for analysis.** Game logs (NDJSON → DuckDB) and data generation are R's domain.
- **C++ first, Rust later.** Engine in C++23. A Rust reimplementation for learning and benchmarking is a future option, not a commitment.

---

## Phases

### Foundation *(done)*

> Change: `project-foundation`

- Meson build system + Catch2
- YAML patch catalog (33 patches, ASCII art shapes) + R codegen → committed C++ header
- Core game state types: `PlayerState` (128-bit), `GameState` (packed shared state)
- Directory structure: `src/`, `tests/`, `data/`, `codegen/`, `logs/`

**Known gap carried forward:** Initial patch circle arrangement (random permutation, fixed at game start) is not yet in `GameState`. R will manage canonical game setups for study.

---

### Game Setup

- `GameSetup`: initial patch circle arrangement (`std::array<uint8_t, 33>`), seeded RNG for reproducibility
- R scripts to generate, store, and version game setups (`data/setups/`)
- C++ code to load and log setups
- Tests

---

### Piece Placement

- Assumption: economy matters more than placement of pieces
- Move to a simplified game state that only tracks patch availability, player positions, buttons, and income — quilt board is reduced to a single integer counting free spaces (0–81)
- Think and decide variants: hard-coded rules or template argument?

---

### Simplified Rules

- Complete game rules under the simple ruleset: legal move generation, move application, terminal detection
- NDJSON game logging (all moves, state transitions, outcomes)
- Random agent (baseline for benchmarking)

---

### Random Sampling Agents

- Uniform random agent (baseline)
- Biased random agent (e.g. prefer cheaper patches, or those with more buttons, or with a better time-per-game-point ratio)

---

### Monte Carlo Tree Search

- MCTS with UCB1
- MCTS + simple rollout policy
- Benchmarking MCTS vs minimax at various time budgets
- R analysis: MCTS convergence, node visit distributions

---

### Heuristic & Search Agents

- Heuristic agent (hand-crafted evaluation function)
- Minimax with alpha-beta pruning
- Iterative deepening
- R analysis: game outcome stats, tile value estimation from game logs

---

### Piece Placement Agent

- Deterministic piece placement agent that chooses the best patch to place based on a heuristic evaluation of the quilt board state
- Expand search to consider piece placement options for the best n positions according to the heuristic for each patch choice, not just patch selection
- For placing multiple pieces, try out all combinations of orderings and placements of the pieces, and pick the best one according to the heuristic evaluation

---

### Reinforcement Learning

- Self-play infrastructure (game loop, data collection)
- Value network + policy network (likely small MLP, no GPU required for Patchwork's state space)
- AlphaZero-style training loop: MCTS guided by learned policy/value
- R analysis: training curves, Elo progression, tile valuation vs. learned values

---

### Rust (Optional)

- Reimplementation of the game engine and state types in Rust
- Benchmark against C++ implementation
- Explore Rust's ownership model for game state mutation patterns

---

## Open Questions / Deferred Decisions

- **Game choice:** Patchwork is the current case study. Whether to generalise to other games (Azul, Cascadia?) is undecided and not a near-term goal.
- **RL framework:** Whether to use an existing RL library or implement from scratch TBD at Phase 5.
- **Rust scope:** Full stack or engine only? Deferred.
- **TUI vs logging:** Simple structured logging (NDJSON) is sufficient; a TUI is not a goal but may emerge naturally.
