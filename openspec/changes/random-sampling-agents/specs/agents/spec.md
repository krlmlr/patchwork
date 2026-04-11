## ADDED Requirements

### Requirement: Agent interface supports selecting a move by strategy name

A free function `select_move(state, setup, rng, strategy)` SHALL dispatch to `random_move` when `strategy == AgentStrategy::Random` and to `biased_random_move` with the appropriate weight function for all other strategies. It SHALL NOT be called on a terminal state.

#### Scenario: Random strategy delegates to uniform random selection

- **WHEN** `select_move` is called with `AgentStrategy::Random`
- **THEN** the returned move is drawn uniformly from `legal_moves(state)` (verified by distribution test matching `random_move`)

#### Scenario: Biased strategy delegates to weighted selection

- **WHEN** `select_move` is called with `AgentStrategy::Cheap`
- **THEN** the returned move distribution matches that of `biased_random_move(..., weight_cheap)`
