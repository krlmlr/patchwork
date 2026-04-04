## ADDED Requirements

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

### Requirement: SimplifiedPlayerState position field supports values up to 63

The position field of `SimplifiedPlayerState` SHALL accept and store any value in 0–63 (full 6-bit range). Values above 53 represent a player who has moved past the last active time square. The existing `SimplifiedPlayerState` 6-bit field already has this capacity; the implementation SHALL NOT cap positions at 53.

#### Scenario: Position stores values above 53

- **WHEN** position is set to any value in 53–63
- **THEN** reading position returns the same value
