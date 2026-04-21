## Why

The uniform random agent (already implemented) serves only as a trivial baseline. To support meaningful benchmarking, heuristic development, and data collection, the engine needs biased sampling agents that prefer game-relevant moves — cheaper patches, high-income patches, or patches with the best income-per-time-cost ratio. These are the lowest-cost agents that exhibit non-trivial strategy and provide a realistic baseline for future MCTS and heuristic agents.

## What Changes

- Add a `biased_random_agent` that selects legal moves by weighted sampling, where each `BuyPatch` move's weight is determined by a caller-supplied weight function; `Advance` receives a configurable advance weight (default `1.0`) passed at call time.
- Provide three built-in weight functions:
  - `weight_cheap`: prefers patches with lower button cost (weight = 1 / (cost + 1))
  - `weight_income`: prefers patches with higher button income
  - `weight_income_per_time`: prefers patches with better income-to-time-cost ratio
- Expose an enum or tag type so the play driver and TUI can select a named bias strategy without depending on raw function pointers.
- Extend the play driver to accept `--agent1`/`--agent2` per-player strategy arguments (defaulting to `random`) and `--advance-weight <w>` to tune how strongly biased agents prefer the advance move (default `1.0`, must be strictly positive).
- Add tests for each weight function, the weighted-sampling distribution, and advance-weight validation.

## Capabilities

### New Capabilities
<!-- None: all new requirements extend existing domains -->

### Modified Capabilities
- `agents`: Extend with requirements for biased random selection, three built-in weight functions (`weight_cheap`, `weight_income`, `weight_income_per_time`), the `AgentStrategy` enum + `make_weight_fn` factory, and the unified `select_move` dispatch function.
- `engine`: Extend play driver with `--agent1`/`--agent2` per-player strategy flags, `--seed1`/`--seed2` per-player seed flags, `--advance-weight` for the advance move's sampling weight, and `"agent_p0"`, `"agent_p1"`, `"seed_p0"`, `"seed_p1"`, `"advance_weight"` fields in the `game_start` NDJSON event.
- `tui`: Extend launch screen to prompt for opponent strategy; add strategy name to frame header; extend `History` to store per-player RNG states for deterministic undo/redo.

## Impact

- New files: `cpp/biased_random_agent.hpp`, `cpp/biased_random_agent.cpp`
- Modified files: `cpp/play_driver.cpp` (add `--agent1`/`--agent2`/`--seed1`/`--seed2`/`--advance-weight` CLI arguments), `cpp/tui_launch.cpp` (add strategy prompt), `cpp/tui_display.hpp` (header shows strategy), `cpp/tui_history.hpp` (per-player RNG states), `openspec/specs/agents/spec.md`, `openspec/specs/engine/spec.md`, `openspec/specs/tui/spec.md`, `docs/glossary.md`
- No changes to game state, move generation, or logging formats beyond the new `game_start` fields
- No new external dependencies
