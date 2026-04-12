## Context

The uniform `random_move` agent exists in `cpp/random_agent.hpp/.cpp` and is used by the play driver for both players. The roadmap's "Random Sampling Agents" phase adds a biased random agent that samples moves according to patch-quality heuristics. The patch catalog (`kPatches`) exposes `buttons`, `time`, and `income` per patch, giving enough signal for simple heuristics.

Current state: `random_move(state, setup, rng)` performs uniform selection over all legal moves. The play driver hard-codes this for both players.

## Goals / Non-Goals

**Goals:**
- Add `biased_random_move(state, setup, rng, weight_fn)` that samples `BuyPatch` moves with weights derived from a caller-supplied function, and `Advance` with a configurable fixed weight.
- Ship three named weight functions: `weight_cheap`, `weight_income`, `weight_income_per_time`.
- Expose a `AgentStrategy` enum (`Random`, `Cheap`, `Income`, `IncomePerTime`) so the play driver and TUI can select a strategy by name.
- Extend the play driver with `--agent1 <strategy>` / `--agent2 <strategy>` for independent per-player strategy selection, and `--seed1 <n>` / `--seed2 <n>` for independent per-player RNG seeds. This enables head-to-head benchmarking and exact game replay from a log.
- Extend the TUI launch screen to let the user choose the opponent agent strategy; extend `History` to store per-player RNG states so that undo/redo is deterministic even when two distinct RNG streams are in play.
- Cover all new code with Catch2 tests.

**Non-Goals:**
- Deterministic/greedy agents (always pick best move) — out of scope for this change.
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

### Decision: Per-player RNG streams

**Chosen:** Each player is assigned an independent `std::mt19937` seeded by `--seed1` and `--seed2` respectively. This makes it possible to replay the exact same game from a log entry: the log records both seeds and strategies, so an external tool (or the TUI) can reconstruct any game state deterministically. It also means a human in the TUI can undo/redo the agent's moves without disturbing the agent's RNG sequence for future moves.

**Alternatives considered:**
- Single shared seed with separate streams derived via seed-splitting: works but couples the two streams; changing one player's strategy would alter the other's outcomes.
- Single shared `std::mt19937` (previous approach): cannot distinguish which player's RNG produced which decision, making replay against a human impossible.

## Risks / Trade-offs

- [Risk] `std::function` wrapping may pessimise tight benchmarking loops → Mitigation: this agent is not on the hot path for MCTS; acceptable for this phase.
- [Risk] Weight functions produce near-zero values for all legal moves, making `std::discrete_distribution` degenerate → Mitigation: assert that at least one weight is positive before sampling; covered by tests.
