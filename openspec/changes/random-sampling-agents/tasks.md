## 1. AgentStrategy enum and factory

- [ ] 1.1 Create `cpp/agent_strategy.hpp` with `AgentStrategy` enum class (`Random`, `Cheap`, `Income`, `IncomePerTime`) and `make_weight_fn(AgentStrategy)` factory returning `std::function<double(const PatchData&)>`
- [ ] 1.2 Add `strategy_name(AgentStrategy)` helper returning the canonical string name (`"random"`, `"cheap"`, `"income"`, `"income-per-time"`) for logging and CLI error messages
- [ ] 1.3 Add `parse_strategy(std::string_view)` helper returning `std::optional<AgentStrategy>` for CLI argument parsing

## 2. Biased random agent implementation

- [ ] 2.1 Create `cpp/biased_random_agent.hpp` declaring `biased_random_move(state, setup, rng, weight_fn, advance_weight)` and the three built-in weight functions (`weight_cheap`, `weight_income`, `weight_income_per_time`)
- [ ] 2.2 Implement `biased_random_agent.cpp` using `std::discrete_distribution` with `Advance` receiving the caller-supplied `advance_weight` parameter; assert `advance_weight > 0` before constructing the distribution
- [ ] 2.3 Implement `weight_cheap` as `1.0 / (patch.buttons + 1)`
- [ ] 2.4 Implement `weight_income` as `static_cast<double>(patch.income + 1)`
- [ ] 2.5 Implement `weight_income_per_time` as `(patch.income + 0.5) / patch.time`
- [ ] 2.6 Add `biased_random_agent.cpp` to `cpp/meson.build`

## 3. Unified select_move dispatch

- [ ] 3.1 Add `select_move(state, setup, rng, strategy, advance_weight)` to a new `cpp/agent.hpp` (not `cpp/agent_strategy.hpp`, which must stay includable without the agent implementations per the "headers compile independently" requirement) that dispatches to `random_move` for `Random` (ignoring `advance_weight`) and to `biased_random_move` with the appropriate weight function and `advance_weight` for all other strategies

## 4. Play driver extension

- [ ] 4.1 Add `--agent1 <strategy>` and `--agent2 <strategy>` argument parsing to `cpp/play_driver.cpp`, each defaulting to `"random"`
- [ ] 4.2 Add `--seed1 <n>` and `--seed2 <n>` argument parsing, each defaulting to `42`; construct two independent `std::mt19937` instances seeded accordingly
- [ ] 4.3 Make `--setup <id>` optional, defaulting to `0`; remove the "missing seed/setup is an error" handling so a bare `patchwork-play` invocation runs a complete game. A malformed value (non-numeric seed, non-numeric or out-of-range setup) still exits non-zero with a stderr usage message
- [ ] 4.4 Add `--advance-weight <w>` argument parsing (default `1.0`); validate that the value is strictly positive on startup and exit non-zero with a stderr error if not
- [ ] 4.5 Replace the hard-coded `random_move` calls in the play driver game loop with per-player `select_move(..., strategy_pN, advance_weight)` using the corresponding RNG instance
- [ ] 4.6 Change the `game_start` NDJSON event to add `"agent_p0"`, `"agent_p1"`, `"seed_p0"`, `"seed_p1"`, `"advance_weight"` fields and drop the single `"seed"` field (superseded by the per-player seeds); keep the `"circle"` field from #26
- [ ] 4.7 Update the `usage()` message to document that all arguments are optional and the new `--agent1`, `--agent2`, `--seed1`, `--seed2`, `--advance-weight` flags
- [ ] 4.8 Validate that an invalid `--agent1` or `--agent2` value prints an error to stderr and exits non-zero

## 5. TUI integration

- [ ] 5.1 Extend the launch screen in `cpp/tui/launch.cpp` to prompt for the opponent agent strategy (after the seed prompt); validate against `parse_strategy` and show an error listing valid names on invalid input
- [ ] 5.2 Add the opponent strategy name to the TUI header section in `cpp/tui/display.cpp`
- [ ] 5.3 Extend `HistoryEntry` in `cpp/tui/history.hpp` to store `rng_p0` and `rng_p1` separately; add `current_rng_p0()` and `current_rng_p1()` accessors
- [ ] 5.4 Update the TUI game loop to pass `rng_p1` to the agent's `select_move` call and restore it from history on undo/redo

## 6. Tests

- [ ] 6.1 Add Catch2 tests for `weight_cheap`, `weight_income`, `weight_income_per_time` using specific `PatchData` values (zero-cost, zero-income, comparative ordering)
- [ ] 6.2 Add distribution test for `biased_random_move` with a weight function that assigns weight 9.0 to one move: verify that move is selected > 60 % of the time over 10 000 calls
- [ ] 6.3 Add reproducibility test: two same-seeded engines produce the same result from `biased_random_move`
- [ ] 6.4 Add advance-weight tests: (a) verify approximately 50/50 split when one BuyPatch (weight 1.0) and one Advance with `advance_weight = 1.0` are the only legal moves; (b) verify the Advance share increases to > 60 % when `advance_weight = 9.0`; (c) verify assertion fires when `advance_weight <= 0`
- [ ] 6.5 Add `select_move` dispatch tests: verify `Random` strategy matches uniform distribution, `Cheap` strategy matches `biased_random_move(..., weight_cheap)` for a fixed seed
- [ ] 6.6 Add `parse_strategy` tests: valid names return correct enum values; invalid name returns `std::nullopt`
- [ ] 6.7 Add `History` tests for per-player RNG: `push` stores both states, `undo`/`redo` restores both states independently, agent move after redo is deterministic
- [ ] 6.8 Run `meson test` and confirm all existing and new tests pass

## 7. Archive specs

- [ ] 7.0 Archive the richer-logging change (#26) **before** this one, so the baseline `game_start` already carries `circle`; this change's `game_start` MODIFIED then rewrites the single `seed` to per-player `seed_p0`/`seed_p1` while preserving `circle` (see design.md cross-PR note)
- [ ] 7.1 Update `openspec/specs/agents/spec.md` by merging the `agents` delta spec
- [ ] 7.2 Update `openspec/specs/engine/spec.md` by merging the `engine` delta spec (optional `--setup`; `--agent1`/`--agent2`/`--seed1`/`--seed2`/`--advance-weight`; `game_start` drops `seed`, adds per-player agent/seed fields + `advance_weight`, keeps `circle`; invalid-argument handling updated for optional args)
- [ ] 7.3 Update `openspec/specs/tui/spec.md` by merging the `tui` delta spec (launch screen strategy prompt, header display, per-player History, updated History unit-test coverage)
