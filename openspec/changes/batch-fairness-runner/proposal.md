## Why

The play driver runs a single game per invocation, so it cannot answer the questions that matter for a stochastic engine: is the simplified game **fair** (no structural first-player advantage), or is its outcome **overly determined by the starting state** (the dealt patch circle)? Answering these needs the *distribution* of outcomes over 100–1000 games, not one sample. Because a full random game costs microseconds, the right primitive is a single-process C++ batch runner that sweeps a `(setup × seed)` grid internally — no per-game process fan-out — feeding an R aggregation step that turns the logs into fairness statistics.

## What Changes

- Add a **C++ batch runner** executable that runs a grid of games in one process: a set of setup ids × a count of agent seeds, each seed derived deterministically from one master seed so the whole batch replays identically.
- Add a **summary output mode**: one NDJSON record per game (`setup_id`, `seed`, `score_p0`, `score_p1`, `winner`, `plies`) instead of full per-move logs, keeping large batches tractable. Full per-move logging remains opt-in.
- Add an **R fairness analysis** that aggregates the batch NDJSON (via DuckDB) into: P1 win rate with a binomial confidence interval, score-margin distribution, a between-setup vs within-setup variance decomposition, and per-setup win-rate spread — with committed plots and tables.
- Reuse the existing single-game loop, `random_agent`, and logger; no changes to game rules or agents. Seat-swapping is intentionally **excluded** — with two identical agents it reproduces the same experiment and adds nothing.

## Capabilities

### New Capabilities
<!-- None: both surfaces fit existing domains per the spec catalog decision rules. -->

### Modified Capabilities

- `engine`: adds a batch-runner executable that runs a `(setup × seed)` grid end-to-end with a deterministic child-seed scheme, and a per-game summary output mode alongside the existing full NDJSON log.
- `analysis`: adds an R pipeline that reads batch game logs into DuckDB and produces fairness statistics (win-rate CI, margin distribution, variance decomposition, per-setup spread) with committed plots and tables.

## Impact

- **New code:** `cpp/batch_driver.cpp` (or similar) + Meson target; `analysis/fairness_analysis.R` + committed outputs under `analysis/output/`.
- **Reused code:** single-game loop from `cpp/play_driver.cpp`, `random_agent`, `game_logger`, `game_setups`, `terminal_and_scoring`.
- **Tests:** Catch2 coverage for reproducibility, grid coverage, and summary-record shape.
- **Specs:** delta files against `engine/spec.md` and `analysis/spec.md`; spec-catalog index unchanged (no new domain).
- **No breaking changes:** the existing play driver, its CLI, and NDJSON event schema are untouched.
