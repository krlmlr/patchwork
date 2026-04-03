## ADDED Requirements

### Requirement: SimplifiedPlayerState fits in 32 bits

`SimplifiedPlayerState` SHALL encode all per-player economy state in no more than 32 bits. Fields SHALL be: free spaces (7 bits, 0–81), time track position (6 bits, 0–53), button balance (7 bits, 0–127), button income (5 bits, 0–31).

#### Scenario: SimplifiedPlayerState size is within bound

- **WHEN** `sizeof(SimplifiedPlayerState)` is evaluated at compile time
- **THEN** the result is less than or equal to 4 bytes

#### Scenario: Default-constructed SimplifiedPlayerState has expected initial values

- **WHEN** a `SimplifiedPlayerState` is default-constructed
- **THEN** free spaces is 81, time position is 0, buttons is 5 (starting value per game rules), income is 0

### Requirement: SimplifiedPlayerState free_spaces field tracks remaining quilt area

The `free_spaces` field of `SimplifiedPlayerState` SHALL represent how many cells of the 9×9 quilt are not yet covered by patches. It SHALL be decremented when a patch is placed; the minimum value is 0.

#### Scenario: free_spaces stores full range

- **WHEN** `free_spaces` is set to any value in 0–81
- **THEN** reading `free_spaces` returns the same value

#### Scenario: free_spaces rejects out-of-range values

- **WHEN** a value greater than 81 is assigned to `free_spaces`
- **THEN** the program either rejects it at compile time or asserts/throws at runtime

### Requirement: SimplifiedPlayerState scalar fields round-trip correctly

All scalar fields of `SimplifiedPlayerState` (time position, buttons, income) SHALL store and retrieve values without loss across their full valid ranges.

#### Scenario: Time position stores full range

- **WHEN** time position is set to any value in 0–53
- **THEN** reading time position returns the same value

#### Scenario: Button balance stores full range

- **WHEN** button balance is set to any value in 0–127
- **THEN** reading buttons returns the same value

#### Scenario: Button income stores full range

- **WHEN** button income is set to any value in 0–31
- **THEN** reading income returns the same value

### Requirement: SimplifiedGameState combines simplified player and shared state

`SimplifiedGameState` SHALL contain two `SimplifiedPlayerState` instances (player 0 and player 1) and shared game state: patch availability bitmask (33 bits), circle marker position (6 bits, 0–32), 7×7 bonus status (2 bits: unclaimed/player0/player1).

#### Scenario: SimplifiedGameState default construction

- **WHEN** a `SimplifiedGameState` is default-constructed
- **THEN** both players have default `SimplifiedPlayerState`, all 33 patches are available, circle marker is 0, bonus is unclaimed

#### Scenario: Patch availability round-trip

- **WHEN** a patch is marked unavailable by index (0–32) in `SimplifiedGameState`
- **THEN** querying that index returns unavailable, and all other patches remain available

#### Scenario: Circle marker stores full range

- **WHEN** circle marker is set to any value in 0–32 in `SimplifiedGameState`
- **THEN** reading it returns the same value

### Requirement: SimplifiedGameState and PlayerState/GameState have no implicit conversions

`SimplifiedGameState` and `GameState` (and their corresponding player-state types) SHALL be distinct types with no implicit or explicit conversions between them.

#### Scenario: Types are distinct

- **WHEN** the type identity of `SimplifiedPlayerState` and `PlayerState` is checked at compile time
- **THEN** `std::is_same_v<SimplifiedPlayerState, PlayerState>` is `false`

### Requirement: Simplified game state types are unit tested

All `SimplifiedPlayerState` and `SimplifiedGameState` fields SHALL have Catch2 unit tests covering default construction, field round-trips, and boundary values.

#### Scenario: Tests exist and pass

- **WHEN** `meson test -C build` is run
- **THEN** all simplified-game-state tests pass with exit code 0
