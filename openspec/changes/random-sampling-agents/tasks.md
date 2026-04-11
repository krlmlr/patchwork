## 1. AgentStrategy enum and factory

- [ ] 1.1 Create `cpp/agent_strategy.hpp` with `AgentStrategy` enum class (`Random`, `Cheap`, `Income`, `IncomePerTime`) and `make_weight_fn(AgentStrategy)` factory returning `std::function<double(const PatchData&)>`
- [ ] 1.2 Add `strategy_name(AgentStrategy)` helper returning the canonical string name (`"random"`, `"cheap"`, `"income"`, `"income-per-time"`) for logging and CLI error messages
- [ ] 1.3 Add `parse_strategy(std::string_view)` helper returning `std::optional<AgentStrategy>` for CLI argument parsing

## 2. Biased random agent implementation

- [ ] 2.1 Create `cpp/biased_random_agent.hpp` declaring `biased_random_move(state, setup, rng, weight_fn)` and the three built-in weight functions (`weight_cheap`, `weight_income`, `weight_income_per_time`)
- [ ] 2.2 Implement `biased_random_agent.cpp` using `std::discrete_distribution` with `Advance` always receiving weight `1.0`
- [ ] 2.3 Implement `weight_cheap` as `1.0 / (patch.buttons + 1)`
- [ ] 2.4 Implement `weight_income` as `static_cast<double>(patch.income + 1)`
- [ ] 2.5 Implement `weight_income_per_time` as `(patch.income + 0.5) / patch.time`
- [ ] 2.6 Add `biased_random_agent.cpp` to `cpp/meson.build`

## 3. Unified select_move dispatch

- [ ] 3.1 Add `select_move(state, setup, rng, strategy)` to `cpp/agent_strategy.hpp` (or a new `cpp/agent.hpp`) that dispatches to `random_move` for `Random` and `biased_random_move` with the appropriate weight function for all other strategies

## 4. Play driver extension

- [ ] 4.1 Add `--agent <strategy>` argument parsing to `cpp/play_driver.cpp`, defaulting to `"random"`
- [ ] 4.2 Replace the hard-coded `random_move` calls in the play driver game loop with `select_move(..., strategy)`
- [ ] 4.3 Add `"agent"` field to the `game_start` NDJSON log event using `strategy_name(strategy)`
- [ ] 4.4 Update the `usage()` message to document the new `--agent` flag
- [ ] 4.5 Validate that an invalid `--agent` value prints an error to stderr and exits non-zero

## 5. Tests

- [ ] 5.1 Add Catch2 tests for `weight_cheap`, `weight_income`, `weight_income_per_time` using specific `PatchData` values (zero-cost, zero-income, comparative ordering)
- [ ] 5.2 Add distribution test for `biased_random_move` with a weight function that assigns weight 9.0 to one move: verify that move is selected > 60 % of the time over 10 000 calls
- [ ] 5.3 Add reproducibility test: two same-seeded engines produce the same result from `biased_random_move`
- [ ] 5.4 Add `Advance`-weight test: verify approximately 50/50 split when one BuyPatch (weight 1.0) and one Advance are the only legal moves
- [ ] 5.5 Add `select_move` dispatch tests: verify `Random` strategy matches uniform distribution, `Cheap` strategy matches `biased_random_move(..., weight_cheap)` for a fixed seed
- [ ] 5.6 Add `parse_strategy` tests: valid names return correct enum values; invalid name returns `std::nullopt`
- [ ] 5.7 Run `meson test` and confirm all existing and new tests pass

## 6. Archive specs

- [ ] 6.1 Update `openspec/specs/agents/spec.md` by merging the `agents` delta spec (add `select_move` requirement)
- [ ] 6.2 Update `openspec/specs/engine/spec.md` by merging the `engine` delta spec (update play driver requirement, add agent logging requirement)
