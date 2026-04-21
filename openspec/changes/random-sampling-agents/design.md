## Context

The uniform `random_move` agent exists in `cpp/random_agent.hpp/.cpp` and is used by the play driver for both players. The roadmap's "Random Sampling Agents" phase adds a biased random agent that samples moves according to patch-quality heuristics. The patch catalog (`kPatches`) exposes `buttons`, `time`, and `income` per patch, giving enough signal for simple heuristics.

Current state: `random_move(state, setup, rng)` performs uniform selection over all legal moves. The play driver hard-codes this for both players.

## Goals / Non-Goals

**Goals:**
- Add `biased_random_move(state, setup, rng, weight_fn, advance_weight)` that samples `BuyPatch` moves with weights derived from a caller-supplied function, and `Advance` with the caller-supplied `advance_weight`.
- Ship three named weight functions: `weight_cheap`, `weight_income`, `weight_income_per_time`.
- Expose a `AgentStrategy` enum (`Random`, `Cheap`, `Income`, `IncomePerTime`) so the play driver and TUI can select a strategy by name.
- Extend the play driver with `--agent1 <strategy>` / `--agent2 <strategy>` for independent per-player strategy selection, `--seed1 <n>` / `--seed2 <n>` for independent per-player RNG seeds, and `--advance-weight <w>` to tune the advance move's sampling weight (default `1.0`, must be strictly positive). This enables head-to-head benchmarking and exact game replay from a log.
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

**Chosen:** `Advance` receives a sampling weight supplied via the `--advance-weight <float>` CLI argument (default `1.0`). This value is forwarded through `select_move` / `biased_random_move` as a `double advance_weight` parameter. The caller is responsible for ensuring the value is strictly positive.

#### Advance weight in the uniform strategy

The existing `random_move` assigns implicit equal weight `1` to every legal move, including `Advance`. So if there are `N` legal moves, `Advance` is selected with probability `1/N`. There is no advance-weight parameter for the uniform strategy; `--advance-weight` is ignored when `--agent1` / `--agent2` is `random`.

#### Advance weight in the biased strategies

For biased strategies the advance weight competes directly with the `BuyPatch` weights. The meaning of `advance_weight = 1.0` depends on the strategy:

| Strategy | BuyPatch weight range | Advance weight | Interpretation |
|---|---|---|---|
| `cheap` | `1/(cost+1)` → `[0.09, 1.0]` for costs 0–10 | configurable (default `1.0`) | Advance is as likely as buying the cheapest (cost-0) patch; the agent becomes progressively more likely to advance as available patches get more expensive. |
| `income` | `income+1` → `[1, income_max+1]` | configurable (default `1.0`) | Advance is as likely as buying a zero-income patch; the agent strongly prefers high-income patches and advances only when all visible patches have low income. |
| `income-per-time` | `(income+0.5)/time` → variable | configurable (default `1.0`) | Advance weight is context-dependent. Typically lower-than-average for patches with good income-per-time ratios, so the agent still advances when patches are time-costly with low income. |

#### Caller guidance

- **Default `1.0`** is numerically safe, keeps `Advance` reachable under all strategies, and produces intuitive behaviour without per-strategy tuning. Recommended for most experiments.
- **Increasing `--advance-weight`** (e.g., `2.0`) makes the agent more likely to pass and accumulate buttons, useful for studying "patience" strategies where waiting for a better buy window is preferred.
- **Decreasing `--advance-weight`** (e.g., `0.5`) pushes the agent to buy more aggressively; ensure the minimum `BuyPatch` weight is also checked so the distribution never degenerates when all patches are affordable.
- **`--advance-weight 0` or negative values** are **forbidden**: if only `Advance` is legal (player cannot afford any patch), the discrete distribution would have all-zero weights and `std::discrete_distribution` would produce undefined behaviour. The implementation validates that the value is strictly positive on start-up and `biased_random_move` asserts that at least one weight is positive before sampling.
- Because both players share the same `--advance-weight` value, experiments that want to compare different advance weights require two separate play driver invocations.

**Alternatives considered:**
- Compile-time constant `kAdvanceWeight`: simple, but prevents experimentation without a rebuild; ruled out by the requirement to support parameter sweeps from the command line.
- Per-player advance weights (`--advance-weight1`, `--advance-weight2`): more flexible, but rare use-case for this phase; can be added later without breaking compatibility.
- Derive advance weight from `max(BuyPatch weights)`: keeps `Advance` always at the same relative probability, but adds runtime overhead and makes the semantics harder to explain.

## CLI Examples

The following examples cover all supported play driver arguments. Defaults: `--seed1 42`, `--seed2 42`, `--agent1 random`, `--agent2 random`, `--setup 0`, `--advance-weight 1.0`, output to stdout.

```sh
# Minimal: uniform random vs. uniform random, setup 0, seeds 42/42
patchwork-play --setup 0

# Reproducible game with explicit seeds
patchwork-play --setup 3 --seed1 1 --seed2 2

# Biased agent vs. uniform random (head-to-head benchmark)
patchwork-play --setup 0 --agent1 cheap --seed1 1 --seed2 2

# Head-to-head biased strategies with explicit seeds
patchwork-play --setup 0 --agent1 income --seed1 100 --agent2 income-per-time --seed2 200

# Tune advance weight: more patient (advancing) agent
patchwork-play --setup 0 --agent1 cheap --advance-weight 2.0 --seed1 1 --seed2 2

# Tune advance weight: more aggressive (buying) agent
patchwork-play --setup 0 --agent1 income --advance-weight 0.25 --seed1 1 --seed2 2

# Same strategies and seeds — log is byte-for-byte identical on every run
patchwork-play --setup 7 --agent1 cheap --seed1 42 --agent2 cheap --seed2 42

# Write log to file instead of stdout
patchwork-play --setup 0 --seed1 1 --seed2 1 --output game.ndjson

# Replay a game exactly from a recorded game_start line:
#   {"event":"game_start","setup":5,"agent_p0":"income","seed_p0":77,"agent_p1":"cheap","seed_p1":88,"advance_weight":1.5}
patchwork-play --setup 5 --agent1 income --seed1 77 --agent2 cheap --seed2 88 --advance-weight 1.5
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
