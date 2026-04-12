## Why

The uniform random agent (already implemented) serves only as a trivial baseline. To support meaningful benchmarking, heuristic development, and data collection, the engine needs biased sampling agents that prefer game-relevant moves — cheaper patches, high-income patches, or patches with the best income-per-time-cost ratio. These are the lowest-cost agents that exhibit non-trivial strategy and provide a realistic baseline for future MCTS and heuristic agents.

## What Changes

- Add a `biased_random_agent` that selects legal moves by weighted sampling, where each `BuyPatch` move's weight is determined by a caller-supplied weight function; `Advance` always receives a fixed weight.
- Provide three built-in weight functions:
  - `weight_cheap`: prefers patches with lower button cost (weight = 1 / (cost + 1))
  - `weight_income`: prefers patches with higher button income
  - `weight_income_per_time`: prefers patches with better income-to-time-cost ratio
- Expose an enum or tag type so the play driver and TUI can select a named bias strategy without depending on raw function pointers.
- Extend the play driver to accept an `--agent <strategy>` argument (or two: `--agent1`, `--agent2`), defaulting to `random` (uniform).
- Add tests for each weight function and the weighted-sampling distribution.

## Capabilities

### New Capabilities
<!-- None: all new requirements extend existing domains -->

### Modified Capabilities
- `agents`: Extend with requirements for biased random selection, three built-in weight functions (`weight_cheap`, `weight_income`, `weight_income_per_time`), the `AgentStrategy` enum + `make_weight_fn` factory, and the unified `select_move` dispatch function.
- `engine`: Extend play driver with the `--agent <strategy>` argument and an `"agent"` field in the `game_start` NDJSON event.

## Impact

- New files: `cpp/biased_random_agent.hpp`, `cpp/biased_random_agent.cpp`
- Modified files: `cpp/play_driver.cpp` (add `--agent` / `--agent1` / `--agent2` CLI argument), `openspec/specs/agents/spec.md`, `openspec/specs/engine/spec.md`
- No changes to game state, move generation, or logging formats
- No new external dependencies
