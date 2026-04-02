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

### Phase 1 — Foundation *(current)*

> Change: `project-foundation`

- Meson build system + Catch2
- YAML patch catalog (33 patches, ASCII art shapes) + R codegen → committed C++ header
- Core game state types: `PlayerState` (128-bit), `GameState` (packed shared state)
- Directory structure: `src/`, `tests/`, `data/`, `codegen/`, `logs/`

**Known gap carried forward:** Initial patch circle arrangement (random permutation, fixed at game start) is not yet in `GameState`. R will manage canonical game setups for study.

---

### Phase 2 — Game Setup & Rules

- `GameSetup`: initial patch circle arrangement (`std::array<uint8_t, 33>`), seeded RNG for reproducibility
- R scripts to generate, store, and version game setups (`data/setups/`)
- Complete game rules: legal move generation, move application, terminal detection
- NDJSON game logging (all moves, state transitions, outcomes)
- Random agent (baseline for benchmarking)

---

### Phase 3 — Heuristic & Search Agents

- Heuristic agent (hand-crafted evaluation function)
- Minimax with alpha-beta pruning
- Iterative deepening
- R analysis: game outcome stats, tile value estimation from game logs

---

### Phase 4 — Monte Carlo Tree Search

- MCTS with UCB1
- MCTS + simple rollout policy
- Benchmarking MCTS vs minimax at various time budgets
- R analysis: MCTS convergence, node visit distributions

---

### Phase 5 — Reinforcement Learning

- Self-play infrastructure (game loop, data collection)
- Value network + policy network (likely small MLP, no GPU required for Patchwork's state space)
- AlphaZero-style training loop: MCTS guided by learned policy/value
- R analysis: training curves, Elo progression, tile valuation vs. learned values

---

### Phase 6 — Rust (Optional)

- Reimplementation of the game engine and state types in Rust
- Benchmark against C++ implementation
- Explore Rust's ownership model for game state mutation patterns

---

## Open Questions / Deferred Decisions

- **Game choice:** Patchwork is the current case study. Whether to generalise to other games (Azul, Cascadia?) is undecided and not a near-term goal.
- **RL framework:** Whether to use an existing RL library or implement from scratch TBD at Phase 5.
- **Rust scope:** Full stack or engine only? Deferred.
- **TUI vs logging:** Simple structured logging (NDJSON) is sufficient; a TUI is not a goal but may emerge naturally.
