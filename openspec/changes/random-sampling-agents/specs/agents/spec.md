## ADDED Requirements

### Requirement: Biased random agent selects a BuyPatch move with probability proportional to its weight

`biased_random_move(state, setup, rng, weight_fn)` SHALL assign a weight to each `BuyPatch` move by calling `weight_fn` with the corresponding `PatchData`, assign the compile-time constant `kAdvanceWeight` (default `1.0`) to the `Advance` move when present, and return a move sampled from `std::discrete_distribution` using those weights. It SHALL assert that at least one weight is positive before sampling. It SHALL NOT be called on a terminal state.

#### Scenario: Selected move is legal

- **WHEN** `biased_random_move` is called on any non-terminal state
- **THEN** the returned move is a member of `legal_moves(state)`

#### Scenario: Higher-weight move is selected more often

- **WHEN** `biased_random_move` is called 10 000 times with a weight function that assigns weight 9.0 to one BuyPatch move and weight 1.0 to all others, using a fixed seed
- **THEN** the high-weight move is selected more than 60 % of the time

#### Scenario: Advance move receives fixed weight 1.0

- **WHEN** the only legal moves are one BuyPatch (weight 1.0 from weight_fn) and one Advance
- **THEN** each is selected approximately 50 % of the time over 10 000 calls (between 40 % and 60 %)

### Requirement: Biased random agent is reproducible given the same seed

Two calls to `biased_random_move` with identically seeded `std::mt19937` instances, the same state, and the same weight function SHALL produce the same move.

#### Scenario: Same seed produces same move

- **WHEN** two `std::mt19937` engines are seeded with the same value
- **THEN** `biased_random_move` returns identical moves when called with those engines on the same state and weight function

### Requirement: Built-in weight function weight_cheap prefers lower button-cost patches

`weight_cheap(patch)` SHALL return `1.0 / (patch.buttons + 1)` so that patches with lower button cost receive higher weight.

#### Scenario: Zero-cost patch has weight 1.0

- **WHEN** `weight_cheap` is called with a patch whose `buttons` field is 0
- **THEN** the return value is 1.0

#### Scenario: Higher-cost patch has lower weight than lower-cost patch

- **WHEN** `weight_cheap` is called with patch A (buttons=2) and patch B (buttons=5)
- **THEN** `weight_cheap(A) > weight_cheap(B)`

### Requirement: Built-in weight function weight_income prefers higher button-income patches

`weight_income(patch)` SHALL return `static_cast<double>(patch.income + 1)` so that patches with higher income receive strictly higher weight than patches with lower income, and zero-income patches still receive positive weight.

#### Scenario: Zero-income patch has positive weight

- **WHEN** `weight_income` is called with a patch whose `income` is 0
- **THEN** the return value is greater than 0.0

#### Scenario: Higher-income patch has higher weight

- **WHEN** `weight_income` is called with patch A (income=1) and patch B (income=3)
- **THEN** `weight_income(B) > weight_income(A)`

### Requirement: Built-in weight function weight_income_per_time prefers better income-to-time-cost ratio

`weight_income_per_time(patch)` SHALL return `(patch.income + 0.5) / patch.time` so that patches with a higher income-per-time-unit receive higher weight, and zero-income patches remain reachable.

#### Scenario: Zero-income patch has positive weight

- **WHEN** `weight_income_per_time` is called with a patch whose `income` is 0
- **THEN** the return value is greater than 0.0

#### Scenario: Same-time patch with higher income has higher weight

- **WHEN** two patches have the same `time` value but different `income` values
- **THEN** the patch with higher income receives a higher weight from `weight_income_per_time`

### Requirement: AgentStrategy enum identifies all supported strategies

A header `cpp/agent_strategy.hpp` SHALL define an enum class `AgentStrategy` with values `Random`, `Cheap`, `Income`, `IncomePerTime`, and a free function `make_weight_fn(AgentStrategy)` that returns a `std::function<double(const PatchData&)>` for each non-`Random` strategy, and returns `nullptr` for `Random`.

#### Scenario: make_weight_fn returns non-null for biased strategies

- **WHEN** `make_weight_fn` is called with `Cheap`, `Income`, or `IncomePerTime`
- **THEN** the returned `std::function` is non-null (truthy when used in a boolean context)

#### Scenario: make_weight_fn returns nullptr for Random

- **WHEN** `make_weight_fn` is called with `AgentStrategy::Random`
- **THEN** the returned `std::function` is null (falsy)

### Requirement: Agent interface supports selecting a move by strategy name

A free function `select_move(state, setup, rng, strategy)` SHALL dispatch to `random_move` when `strategy == AgentStrategy::Random` and to `biased_random_move` with the appropriate weight function for all other strategies. It SHALL NOT be called on a terminal state.

#### Scenario: Random strategy delegates to uniform random selection

- **WHEN** `select_move` is called with `AgentStrategy::Random`
- **THEN** the returned move is drawn uniformly from `legal_moves(state)` (verified by distribution test matching `random_move`)

#### Scenario: Biased strategy delegates to weighted selection

- **WHEN** `select_move` is called with `AgentStrategy::Cheap`
- **THEN** the returned move distribution matches that of `biased_random_move(..., weight_cheap)`

### Requirement: Biased random agent compiles and links as a standalone usable unit

`biased_random_agent.hpp` and `agent_strategy.hpp` SHALL be includable in a translation unit that does not include the logger or play driver, without compilation error.

#### Scenario: Headers compile independently

- **WHEN** `biased_random_agent.hpp` or `agent_strategy.hpp` is included without other patchwork headers
- **THEN** the translation unit compiles without error
