# Patchwork Engine — Roadmap

> **Maintenance note:** This file is human-maintained and AI-read.
> Humans update phases, priorities, and open questions here.
> AI reads this file to understand current project state before proposing or implementing changes.

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
- Directory structure: `cpp/`, `tests/`, `data/`, `codegen/`, `logs/`

**Known gap carried forward:** Initial patch circle arrangement (random permutation, fixed at game start) is not yet in `GameState`. R will manage canonical game setups for study.

---

### Game Setup *(done)*

- `GameSetup`: initial patch circle arrangement (`std::array<uint8_t, 33>`), seeded RNG for reproducibility
- R scripts to generate, store, and version game setups (`data/setups/`)
- C++ code to load and log setups
- Tests

---

### Start Without Piece Placement *(done)*

- Assumption: economy matters more than placement of pieces
- Move to a simplified game state that only tracks patch availability, player positions, buttons, and income — quilt board is reduced to a single integer counting free spaces (0–81)
- Think and decide variants: hard-coded rules or template argument?

---

### Simplified Rules *(done)*

> Change: `simplified-rules`

- Complete game rules under the simple ruleset: legal move generation, move application, terminal detection
- Distinguish between phases: tile picking + advance + payout while allowed, then tile placement; tile picking is legal as long as buttons and time allows, tile placement may still be impossible due to board state
- Award 1x1 leather patches at the correct thresholds
- Implement the 7×7 bonus scoring rules (simple version: bonus is claimed by the first player to reach 7×8 = 56 occupied tiles)
- NDJSON game logging (all moves, state transitions, outcomes)
- Random agent (baseline for benchmarking)
- Reproducible random play with logging, random seed and starting position as input

---

### Tile Analysis

- R analysis of tile value and time cost distributions from the patch catalog
- Model for gain-per-time of each patch, also depending on shape (circumference vs. area or other shape features)
- Total gain depending on current time-track position (early-game vs late-game value)
- Plots and tables to inform heuristic design and future agent development

---

### Richer Logging

- Add full game setup to logs (initial patch circle arrangement, seed)
- Add more post-move state summaries to move logs (income, free spaces, total board value, patch circle)

---

### Fast CI/CD Setup *(done)*

- Install R packages from PPM
- For GHA, Copilot and devcontainer

---

### TUI / Interactive Mode *(done)*

- Simple text-based UI for playing against a human
- Display game state in ASCII art (buttons, income, free spaces, time track with player positions, patch circle)
- Display scrolling log of moves and events
- Input handling for move selection (buy patch by index, or advance) and other commands, single keypress preferred
- Unlimited undo/redo for testing and exploration
- Extensible for full quilt board display in the future
- Choose game engine, configuration and seed from the TUI
- Show a preview for the TUI as part of the design

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

### TUI refinements

- Add TUI elements to glossary
- If a keypress is detected but leads to no action or is otherwise unavailable, show reason in red over the last line of the TUI (e.g. "not enough buttons", "patch not available", "already at starting position")
- When game is finished, show final scores and declare winner in green over the last line, undo remains available
- Gameplay bug: default game setup 0/seed 42, P1 buys L then advances twice then buys w then tries to buy A (cost 0), this is marked as allowed in the highlighting of the circle but fails (pressing 3 leads to no action). Snapshot:

    ```txt
    ┌ PATCHWORK -- seed ? / setup 0 --─────────────────────────────────────────────────── ▶ P1 ─┐
    │ Circle: Zt.v3....Jq.SOAXox41TzpedlsNHjm52                                                 │
    │                     ^                                                                     │
    │ [ 1] S  cost  7  time  6  inc 3                   [1/2/3]buy  [a]adv      [q]quit         │
    │ [ 2] O  cost  5  time  3  inc 1                   [z/u]undo [Z/r]redo   [</>]log  [w]wrap │
    │ [ 3] A  cost  0  time  3  inc 1                   [m]v [f]^ [h]^/2  [,]- [.]+             │
    │ [ 4] X  cost  1  time  4  inc 1                                                           │
    │ [ 5] o  cost  6  time  5  inc 2                                                           │
    │ [ 6] x  cost  5  time  4  inc 2                                                           │
    │ [ 7] 4  cost  3  time  3  inc 1                                                           │
    │ [ 8] 1  cost  7  time  2  inc 2                                                           │
    │ [ 9] T  cost  5  time  5  inc 2                                                           │
    │ [10] z  cost  2  time  3  inc 1                                                           │
    │ [11] p  cost  2  time  2  inc 0                                                           │
    │ [12] e  cost  3  time  6  inc 2                                                           │
    ├─────────────────────────────────────────────┬─────────────────────────────────────────────┤
    │ P1  btn   1  inc  5  pos 16  fr 72          │ P2  btn   7  inc  2  pos 18  fr 59          │
    ├───────────┬───────────┬─────────────────────┴─────────────────────────────────────────────┤
    │ P1 quilt  │ P2 quilt  │ Event log                                                         │
    │ ????????? │ ????????? │ > P2 (cpu) bought [k]                                             │
    │ ????????? │ ????????? │ > P2 (cpu) bought [u]                                             │
    │ ????????? │ ????????? │ > P2 (cpu) advanced                                               │
    │ ????????? │ ????????? │ > P1 advanced                                                     │
    │ ????????? │ ????????? │ > P2 (cpu) bought [y]                                             │
    │ ????????? │ ????????? │ > P1 advanced                                                     │
    │ ????????? │ ????????? │ > P2 (cpu) advanced                                               │
    │ ????????? │ ????????? │ > P1 bought [w]                                                   │
    │ ????????? │ ????????? │ > P2 (cpu) bought [U]                                             │
    ├───────────┴───────────┴─ ndjson log (9 lines) ──────────────────────[m]v [f]^ [h]^/2 [,.]─┤
    │ {"event":"move","ply":26,"player":1,"move_type":"buy_patch","patch_index":23,"position"   │
    │ {"event":"move","ply":27,"player":1,"move_type":"buy_patch","patch_index":11,"position"   │
    │ {"event":"move","ply":28,"player":1,"move_type":"advance","position":7,"buttons":6}       │
    │ {"event":"move","ply":29,"player":0,"move_type":"advance","position":8,"buttons":5}       │
    │ {"event":"move","ply":30,"player":1,"move_type":"buy_patch","patch_index":14,"position"   │
    │ {"event":"move","ply":31,"player":0,"move_type":"advance","position":12,"buttons":11}     │
    │ {"event":"move","ply":32,"player":1,"move_type":"advance","position":13,"buttons":6}      │
    │ {"event":"move","ply":33,"player":0,"move_type":"buy_patch","patch_index":18,"position"   │
    │ {"event":"move","ply":34,"player":1,"move_type":"buy_patch","patch_index":21,"position"   │
    └───────────────────────────────────────────────────────────────────────────────────────────┘
    ```

- NDJSON log scrolling does not work yet: horizontal broken (scrolls game/event log instead), vertical unspecified
- NDJSON log word wrapping broken
- Each rendered frame adds to the terminal's output history; is there a way to avoid this and keep the terminal's scrollback clean?
- Start in `NdjsonSemiMaximize` mode
- Effectively unbounded event and JSON log, no truncation (games with many moves consist only of advances, adjust the max log size to accommodate)
- Display bonus tile status and leather patch counts in the TUI
- Display more stats in the TUI (current score, projected score, spaces covered + placement gain + projected income per patch, ...)
- Circle lines outside buying window should show as faded
- No winner when quitting early

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
