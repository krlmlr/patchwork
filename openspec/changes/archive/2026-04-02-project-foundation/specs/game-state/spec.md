## ADDED Requirements

### Requirement: PlayerState fits in 128 bits

`PlayerState` SHALL encode all per-player game state in no more than 128 bits (16 bytes). Fields SHALL be: quilt board (81 bits, 9×9 grid), time track position (6 bits, 0–53), button balance (7 bits, 0–127), button income (5 bits, 0–31).

#### Scenario: PlayerState size is within bound

- **WHEN** `sizeof(PlayerState)` is evaluated at compile time
- **THEN** the result is less than or equal to 16 bytes

#### Scenario: Default-constructed PlayerState is empty

- **WHEN** a `PlayerState` is default-constructed
- **THEN** the board is empty (all cells false), position is 0, buttons is 5 (starting value per game rules), income is 0

### Requirement: PlayerState board field is a 9×9 bit grid

The board field of `PlayerState` SHALL represent which cells of the 9×9 quilt are occupied. Individual cells SHALL be addressable by (row, col) coordinates.

#### Scenario: Cell set and get round-trip

- **WHEN** cell (row, col) is set to occupied on a `PlayerState`
- **THEN** querying that cell returns occupied, and all other cells remain unoccupied

#### Scenario: Board rejects out-of-bounds coordinates

- **WHEN** a cell outside the 9×9 grid is accessed
- **THEN** the program either rejects it at compile time or throws/asserts at runtime

### Requirement: PlayerState fields round-trip correctly

All scalar fields of `PlayerState` (position, buttons, income) SHALL store and retrieve values without loss across their full valid ranges.

#### Scenario: Position stores full range

- **WHEN** position is set to any value in 0–53
- **THEN** reading position returns the same value

#### Scenario: Buttons stores full range

- **WHEN** button balance is set to any value in 0–127
- **THEN** reading buttons returns the same value

#### Scenario: Income stores full range

- **WHEN** button income is set to any value in 0–31
- **THEN** reading income returns the same value

### Requirement: GameState combines player and shared state

`GameState` SHALL contain two `PlayerState` instances (player 0 and player 1) and shared game state: patch availability bitmask (33 bits), circle marker position (6 bits, 0–32), 7×7 bonus status (2 bits: unclaimed/player0/player1).

#### Scenario: GameState default construction

- **WHEN** a `GameState` is default-constructed
- **THEN** both players have default `PlayerState`, all 33 patches are available, circle marker is 0, bonus is unclaimed

#### Scenario: Patch availability round-trip

- **WHEN** a patch is marked unavailable by index (0–32)
- **THEN** querying that index returns unavailable, and all other patches remain available

#### Scenario: Circle marker stores full range

- **WHEN** circle marker is set to any value in 0–32
- **THEN** reading it returns the same value

### Requirement: Game state types are unit tested

All `PlayerState` and `GameState` fields SHALL have Catch2 unit tests covering default construction, field round-trips, and boundary values.

#### Scenario: Tests exist and pass

- **WHEN** `meson test -C build` is run
- **THEN** all game state tests pass with exit code 0
