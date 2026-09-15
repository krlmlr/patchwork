## Context

The engine can play one game per process invocation (`cpp/play_driver.cpp`): fixed `--seed`, fixed `--setup`, emit NDJSON, exit. The `game_end` event already carries `score_p0`, `score_p1`, and `winner`, so everything a fairness study needs is present per game — what's missing is (a) a way to run *many* games cheaply and reproducibly, and (b) an aggregation layer that turns the resulting logs into fairness statistics.

Two questions motivate the work, and they load different knobs:

- **Fairness** is a *mean* effect. With two identical uniform-random agents, neither contributes any asymmetry, so a P1 win rate or mean score margin that departs from 50 % / 0 is a structural first-player advantage of the rules + setup.
- **Setup-dependence** is a *variance-decomposition* effect: `Var(outcome) = Var_between_setups + Var_within_setup(agent RNG)`. A large between-setup share means the dealt patch circle largely fixes the result regardless of play.

Constraints: C++23 + Meson + Catch2; reproducibility is a stated project principle; "R for analysis" (NDJSON → DuckDB) is the canonical analysis path; game rules and agents are out of scope for this change.

## Goals / Non-Goals

**Goals:**

- A single-process C++ batch runner that sweeps a `(setup × seed)` grid and emits results, reusing the existing game loop, `random_agent`, and logger.
- Deterministic child-seed derivation from one master seed, so an entire batch replays byte-for-byte.
- A compact per-game summary output mode, with full per-move NDJSON opt-in.
- An R fairness analysis producing win-rate CI, margin distribution, variance decomposition, and per-setup spread as committed plots/tables.

**Non-Goals:**

- No changes to game rules, scoring, agents, or the existing single-game play driver.
- No seat-swapping — with identical agents it reproduces the same experiment (belongs to the later different-agent benchmarking phase).
- No external process-level parallelism (GNU parallel / targets); the internal loop is fast enough for random agents. Revisit only when expensive agents (MCTS) arrive.
- No new agent types (uniform-random only).

## Decisions

### Batch runner is a single process with an internal loop

A full random game is microseconds; spawning a process (or a build-system branch) per game would be almost entirely fan-out overhead. So one executable loops over the grid in-process and streams results to one output stream. **Alternative — one process per game via GNU parallel / a targets DAG:** rejected for this phase because orchestration cost dwarfs game cost; reserved for the expensive-agent phase where per-game work justifies it. The loop is deliberately factored so it stays reusable when parallelism is later warranted.

### CLI: grid over setups × seed count, one master seed

The runner accepts a set of setup ids, a game count `N` (seeds per setup), and a single `--master-seed`. Example shape: `--setups 0,1,2` (or a range) `--games 1000 --master-seed 42 [--summary|--full] [--output <file>]`. This keeps the "same game, many runs" case (`--setups 0 --games 1000`) and the cross-setup sweep (`--setups 0..K --games 1000`) as one tool.

### Deterministic child seeds via SplitMix64 over (master_seed, setup_id, game_index)

Each game's `std::mt19937` is seeded from a hash of `(master_seed, setup_id, game_index)` (SplitMix64 mixing), **not** from a single shared stream advanced game-to-game. Rationale: order-independent and grid-shape-independent — game *(setup 0, index 7)* gets the same seed whether or not other setups are in the sweep, so partial and re-ordered batches stay reproducible and directly comparable. **Alternative — one long RNG stream reseeding sequentially:** rejected because results would depend on grid ordering and size. The derived per-game seed is recorded in each output record so any single game can be replayed with the existing play driver.

### Summary mode is the default output; full NDJSON is opt-in

Summary mode emits one record per game: `{event:"game_summary", setup_id, seed, score_p0, score_p1, winner, plies}`. `plies` is the move counter already tracked by the loop. 1000 games × hundreds of plies of full move logs is large and unnecessary for fairness aggregation, which only reads terminal outcomes. `--full` restores per-game full NDJSON (game_start / move / game_end) for debugging a specific run. The `game_summary` record is additive to the engine's NDJSON schema; existing events are unchanged.

### R aggregation reads summary NDJSON via DuckDB

`analysis/fairness_analysis.R` ingests a batch summary file into DuckDB and computes: P1 win rate + Wilson/exact binomial CI; score-margin (`score_p1 − score_p0`) distribution and mean CI; variance decomposition (group by `setup_id` → between-group vs within-group variance of the margin, i.e. a one-way ANOVA-style split); per-setup win-rate table with per-setup CIs. Outputs committed under `analysis/output/` (plots + CSV), matching the existing tile-analysis convention (`pkgload::load_all()`, idempotent, documented deps).

## Risks / Trade-offs

- **Identical agents can't reveal skill-driven balance, only structural balance** → That is exactly the intended scope; documented as a Non-Goal, and the roadmap defers agent-vs-agent (with seat-swap) to the MCTS phase.
- **A tiny legal-move space could make random play near-deterministic, masking variance** → the variance decomposition surfaces this directly (near-zero within-setup variance is itself the finding); game-length distribution is logged as a sanity check.
- **DuckDB as an analysis dependency** → it is already the project's stated NDJSON analysis path; declare it in `DESCRIPTION` / a top-of-script comment as with the existing analysis script.
- **Summary schema drift vs full logs** → `score_p0/score_p1/winner` field names mirror the existing `game_end` event so the two paths stay consistent; covered by a test.

## Open Questions

- Exact CLI surface for the setup set — explicit list (`--setups 0,1,2`) vs range (`--setups 0-9`) vs both. Lean toward accepting both; final form settled during implementation.
- How many setups constitute a meaningful cross-setup sample for the variance decomposition — a data question for the analysis, not a blocker for the runner.
