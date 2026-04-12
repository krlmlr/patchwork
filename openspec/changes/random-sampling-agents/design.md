## Context

The uniform `random_move` agent exists in `cpp/random_agent.hpp/.cpp` and is used by the play driver for both players. The roadmap's "Random Sampling Agents" phase adds a biased random agent that samples moves according to patch-quality heuristics. The patch catalog (`kPatches`) exposes `buttons`, `time`, and `income` per patch, giving enough signal for simple heuristics.

Current state: `random_move(state, setup, rng)` performs uniform selection over all legal moves. The play driver hard-codes this for both players.

## Goals / Non-Goals

**Goals:**
- Add `biased_random_move(state, setup, rng, weight_fn)` that samples `BuyPatch` moves with weights derived from a caller-supplied function, and `Advance` with a configurable fixed weight.
- Ship three named weight functions: `weight_cheap`, `weight_income`, `weight_income_per_time`.
- Expose a `AgentStrategy` enum (`Random`, `Cheap`, `Income`, `IncomePerTime`) so the play driver and TUI can select a strategy by name.
- Extend the play driver with `--agent <strategy>` (applies to both players).
- Cover all new code with Catch2 tests.

**Non-Goals:**
- `--agent1`/`--agent2` per-player strategy flags — a symmetric `--agent` is sufficient for this benchmarking phase; asymmetric flags can be added when the analysis phase requires head-to-head comparisons.
- `--seed1`/`--seed2` per-player seed flags — a single seed drives one `std::mt19937` shared between both players; this already provides full reproducibility and allows sweeping seeds in R analysis. Independent per-agent streams can always be derived from the master seed in code if needed.
- Deterministic/greedy agents (always pick best move) — out of scope for this change.
- Per-player agent type in NDJSON log format — deferred; logs will record strategy name as an informational field in `game_start` only.
- Shape-aware heuristics (requires full quilt board) — deferred to Piece Placement Agent phase.

## Decisions

### Decision: Weight function as a `std::function` parameter vs. enum dispatch

**Chosen:** Expose both. The `biased_random_move` free function accepts a `std::function<double(const PatchData&)>` so it is maximally testable and reusable. A thin `make_weight_fn(AgentStrategy)` factory returns the appropriate `std::function` for play driver and TUI use.

**Alternatives considered:**
- Template parameter (avoids `std::function` overhead): adds complexity at call sites and makes the factory less straightforward; overhead is negligible at game-loop speeds.
- Enum-only dispatch inside `biased_random_move`: harder to test weight logic in isolation.

### Decision: Weight for `Advance` moves

**Chosen:** `Advance` receives a fixed weight of `1.0` regardless of strategy. This means biased agents still advance when they cannot afford any patch or when advancing is the only legal move — correct game-rule behaviour. The weight is a compile-time constant in `biased_random_agent.hpp`, not a runtime parameter, to keep the interface simple.

**Alternatives considered:**
- Weight of 0 for `Advance`: would cause assertion failure when only `Advance` is legal.
- Configurable advance weight: unnecessary complexity for this phase.

### Decision: `weight_cheap` formula

**Chosen:** `1.0 / (cost + 1)` where `cost = patch.buttons`. Cost 0 → weight 1.0, cost 1 → 0.5, cost 10 → ~0.09. The `+1` prevents division by zero for free patches.

### Decision: `weight_income_per_time` formula

**Chosen:** `(income + 0.5) / time` where `income = patch.income` and `time = patch.time`. The `+0.5` additive smoothing avoids zero-weight for 0-income patches and keeps them reachable. Patches with `time == 0` do not exist in the catalog, so no guard is needed, but a static assert in tests will verify this.

### Decision: File layout

**Chosen:** `cpp/biased_random_agent.hpp` / `cpp/biased_random_agent.cpp`, mirroring the existing random agent. `AgentStrategy` enum lives in `cpp/agent_strategy.hpp` (a header-only enum + factory) so the play driver, TUI, and tests can include it without pulling in implementation.

## Risks / Trade-offs

- [Risk] `std::function` wrapping may pessimise tight benchmarking loops → Mitigation: this agent is not on the hot path for MCTS; acceptable for this phase.
- [Risk] Adding `--agent1`/`--agent2` complicates CLI parsing → Mitigation: keep `--agent` as a single symmetric default; add asymmetric flags only if needed.
- [Risk] Weight functions produce near-zero values for all legal moves, making `std::discrete_distribution` degenerate → Mitigation: assert that at least one weight is positive before sampling; covered by tests.
