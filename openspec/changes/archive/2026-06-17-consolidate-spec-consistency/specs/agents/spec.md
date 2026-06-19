## MODIFIED Requirements

### Requirement: Random agent selects a legal move uniformly at random

`random_move(state, setup, rng)` SHALL select and return one move from the collection returned by `legal_moves(state, setup)` with equal probability for each legal move. The `setup` parameter (a `const GameSetup&`) is required because legal-move generation depends on the patch circle. The caller SHALL NOT invoke `random_move` on a terminal state.

#### Scenario: Selected move is legal

- **WHEN** `random_move` is called on any non-terminal state
- **THEN** the returned move is a member of `legal_moves(state, setup)`

#### Scenario: Distribution is approximately uniform over many samples

- **WHEN** `random_move` is called 10 000 times on a state with 3 legal moves, using a fixed seed
- **THEN** each move is selected between 25 % and 42 % of the time (statistical tolerance)
