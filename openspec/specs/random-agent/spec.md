### Requirement: Random agent selects a legal move uniformly at random

`random_move(state, rng)` SHALL select and return one move from the collection returned by `legal_moves(state)` with equal probability for each legal move. It SHALL NOT call `legal_moves` when the state is terminal.

#### Scenario: Selected move is legal

- **WHEN** `random_move` is called on any non-terminal state
- **THEN** the returned move is a member of `legal_moves(state)`

#### Scenario: Distribution is approximately uniform over many samples

- **WHEN** `random_move` is called 10 000 times on a state with 3 legal moves, using a fixed seed
- **THEN** each move is selected between 25 % and 42 % of the time (statistical tolerance)

### Requirement: Random agent is reproducible given the same seed

Two calls to `random_move` with identically seeded `std::mt19937` instances and the same state SHALL produce the same move.

#### Scenario: Same seed produces same move

- **WHEN** two `std::mt19937` engines are seeded with the same value
- **THEN** `random_move` returns identical moves when called with those engines on the same state

### Requirement: Random agent compiles and links as a standalone usable unit

The random agent SHALL be declared in a header under `cpp/` and implemented in a corresponding `.cpp` file. It SHALL have no dependency on the logger or play driver.

#### Scenario: Random agent header can be included independently

- **WHEN** `random_agent.hpp` is included in a translation unit that does not include the logger
- **THEN** the code compiles without error
