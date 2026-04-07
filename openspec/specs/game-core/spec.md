# Game Core Specification

## Purpose
Defines the fundamental data types that represent game state, plus the initial circular arrangement of patches at the start of a game. These are the nouns of the system — what the state looks like, not how it changes.

## Requirements

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

### Requirement: SimplifiedGameState tracks which player acts next

`SimplifiedGameState` SHALL store a 1-bit `next_player` field (0 or 1) in the existing shared `uint64_t` word (bit 41, currently unused). It SHALL default to 0 (player 0 goes first). `active_player()` SHALL return this field. `apply_move` SHALL update it: if the moved player's new position strictly exceeds the opponent's position, the opponent becomes `next_player`; otherwise the moved player remains `next_player`.

#### Scenario: next_player defaults to 0

- **WHEN** a `SimplifiedGameState` is default-constructed
- **THEN** `active_player()` returns 0

#### Scenario: next_player round-trips

- **WHEN** `next_player` is set to 1
- **THEN** `active_player()` returns 1

#### Scenario: Active player remains active on tie

- **WHEN** player 0 is active, moves from position 4 to position 8, and player 1 is at position 8
- **THEN** `active_player()` returns 0 in the successor state

#### Scenario: Active player switches when overtaking opponent

- **WHEN** player 0 is active, moves from position 4 to position 9, and player 1 is at position 8
- **THEN** `active_player()` returns 1 in the successor state

### Requirement: SimplifiedGameState tracks which player first reached the terminal position

`SimplifiedGameState` SHALL store a 1-bit `first_to_finish` field (0 or 1) in the existing shared `uint64_t` word (bit 42, currently unused). It SHALL default to 0. `apply_move` SHALL set it exactly once: the first time a player's position transitions from < 53 to ≥ 53 while the other player's position is still < 53. Once set, it SHALL NOT be changed. `winner` reads this field to resolve equal scores.

#### Scenario: first_to_finish defaults to 0

- **WHEN** a `SimplifiedGameState` is default-constructed
- **THEN** `first_to_finish` is 0

#### Scenario: first_to_finish set when first player reaches 53

- **WHEN** player 0 is at position 52 and player 1 is at position 40, and a move advances player 0 to position 53
- **THEN** `first_to_finish` is set to 0

#### Scenario: first_to_finish not overwritten when second player reaches 53

- **WHEN** `first_to_finish` is already set to 0 and a subsequent move advances player 1 to position 53
- **THEN** `first_to_finish` remains 0

The position field of `SimplifiedPlayerState` SHALL accept and store any value in 0–63 (full 6-bit range). Values above 53 represent a player who has moved past the last active time square. The existing `SimplifiedPlayerState` 6-bit field already has this capacity; the implementation SHALL NOT cap positions at 53.

#### Scenario: Position stores values above 53

- **WHEN** position is set to any value in 53–63
- **THEN** reading position returns the same value

### Requirement: GameSetup encodes the initial patch circle arrangement

`GameSetup` SHALL hold the initial circular arrangement of all 33 patches as a `std::array<uint8_t, 33>` of integer patch IDs (0–32). The constructor SHALL accept a 33-character `std::string_view` of single-char patch names and convert each character to its integer ID by looking up the patch catalog.

#### Scenario: Construction from string produces a valid permutation

- **WHEN** `GameSetup` is constructed from a 33-char string of patch names
- **THEN** the internal circle contains each of the integer IDs `0` through `32` exactly once

#### Scenario: Last tile in every canonical setup is the two-square patch

- **WHEN** any canonical setup string from `patchwork::kGameSetups` is examined
- **THEN** the last character (position 32) is `'2'` — the name of the two-square tile

### Requirement: GameSetup serialises to an NDJSON record

`GameSetup::to_ndjson` SHALL emit exactly one JSON line to the provided `std::ostream` containing the type tag and circle as a 33-character string.

#### Scenario: NDJSON output is a single line with correct fields

- **WHEN** `to_ndjson(out)` is called on a `GameSetup`
- **THEN** the output contains a single newline-terminated JSON object
- **AND** the object has `"type": "setup"` and `"circle": "<33-char-string>"`

### Requirement: Canonical setups are embedded in `cpp/generated/game_setups.hpp`

The repository SHALL contain a committed generated header `cpp/generated/game_setups.hpp` produced by `codegen/generate_setups.R`, defining a `constexpr std::array<std::string_view, kNumGameSetups>` named `kGameSetups` in namespace `patchwork`, where `kNumGameSetups` is a named constant matching the value hard-coded in the R script (initially `100`). Generating more setups in future SHALL leave the first 100 entries unchanged.

#### Scenario: Generated header is present and contains 100 entries

- **WHEN** the repository is cloned
- **THEN** `cpp/generated/game_setups.hpp` exists and `patchwork::kGameSetups` has exactly 100 entries

#### Scenario: Each entry is a valid permutation of patch names

- **WHEN** any entry in `patchwork::kGameSetups` is examined
- **THEN** it is exactly 33 characters
- **AND** it contains each of the 33 single-char patch names exactly once

### Requirement: GameSetup is unit tested

All `GameSetup` behaviours SHALL have Catch2 unit tests covering construction from string, access via `kGameSetups`, and `to_ndjson`.

#### Scenario: Tests exist and pass

- **WHEN** `meson test -C build` is run
- **THEN** all game-setup tests pass with exit code 0
