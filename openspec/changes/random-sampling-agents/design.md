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

**Chosen:** `Advance` receives a fixed weight of `1.0` for all biased strategies (exposed as a named constant `kAdvanceWeight = 1.0` in `biased_random_agent.hpp`). The value is not a runtime parameter in this phase — callers wishing to experiment should change the constant and rebuild.

#### Advance weight in the uniform strategy

The existing `random_move` assigns implicit equal weight `1` to every legal move, including `Advance`. So if there are `N` legal moves, `Advance` is selected with probability `1/N`. There is no free parameter here.

#### Advance weight in the biased strategies

For biased strategies the `Advance` weight competes directly with the `BuyPatch` weights. The meaning of `kAdvanceWeight = 1.0` depends on the strategy:

| Strategy | BuyPatch weight range | Advance weight | Interpretation |
|---|---|---|---|
| `cheap` | `1/(cost+1)` → `[0.09, 1.0]` for costs 0–10 | `1.0` | Advance is as likely as buying the cheapest (cost-0) patch; the agent becomes progressively more likely to advance as available patches get more expensive. |
| `income` | `income+1` → `[1, income_max+1]` | `1.0` | Advance is as likely as buying a zero-income patch; the agent strongly prefers high-income patches and advances only when all visible patches have low income. |
| `income-per-time` | `(income+0.5)/time` → variable | `1.0` | Advance weight is context-dependent. Typically lower-than-average for patches with good income-per-time ratios, so the agent still advances when patches are time-costly with low income. |

#### Caller guidance

- **Increasing `kAdvanceWeight`** (e.g., to `2.0`) makes the agent more likely to pass and spend buttons, useful for studying "patience" strategies where waiting for a better buy window is preferred.
- **Decreasing `kAdvanceWeight`** (e.g., to `0.5`) pushes the agent to buy more aggressively; ensure the minimum `BuyPatch` weight is also checked so the distribution never degenerates when all patches are affordable.
- **Setting `kAdvanceWeight = 0`** is **forbidden**: if only `Advance` is legal (player cannot afford any patch), the discrete distribution would have all-zero weights and `std::discrete_distribution` would produce undefined behaviour. The implementation asserts that at least one weight is positive.
- For this phase, `1.0` is the recommended default: it is numerically safe, keeps `Advance` reachable under all strategies, and produces intuitive behaviour without per-strategy tuning.

**Alternatives considered:**
- Weight of 0 for `Advance`: forbidden — causes assertion failure when only `Advance` is legal.
- Per-strategy advance weights (e.g., `0.5` for `cheap`, `2.0` for `income`): more expressive but requires additional API surface; deferred to a future parameter-sweep phase.
- Derive advance weight from `max(BuyPatch weights)`: keeps `Advance` always at the same relative probability, but adds runtime overhead and makes the semantics harder to explain.

## CLI Examples

The following examples cover all supported play driver arguments. Defaults: `--seed1 42`, `--seed2 42`, `--agent1 random`, `--agent2 random`, `--setup 0`, output to stdout.

```sh
# Minimal: uniform random vs. uniform random, setup 0, seeds 42/42
patchwork-play --setup 0

# Reproducible game with explicit seeds
patchwork-play --setup 3 --seed1 1 --seed2 2

# Biased agent vs. uniform random (head-to-head benchmark)
patchwork-play --setup 0 --agent1 cheap --seed1 1 --seed2 2

# Head-to-head biased strategies with explicit seeds
patchwork-play --setup 0 --agent1 income --seed1 100 --agent2 income-per-time --seed2 200

# Same strategies and seeds — log is byte-for-byte identical on every run
patchwork-play --setup 7 --agent1 cheap --seed1 42 --agent2 cheap --seed2 42

# Write log to file instead of stdout
patchwork-play --setup 0 --seed1 1 --seed2 1 --output game.ndjson

# Replay a game exactly from a recorded game_start line:
#   {"event":"game_start","setup":5,"agent_p0":"income","seed_p0":77,"agent_p1":"cheap","seed_p1":88}
patchwork-play --setup 5 --agent1 income --seed1 77 --agent2 cheap --seed2 88
```

The TUI uses the same strategy names at the launch-screen prompt:

```
Setup index [0-99, default 0]: 3
Seed for opponent [default 42]: 1
Opponent strategy [random/cheap/income/income-per-time, default random]: cheap
```

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
