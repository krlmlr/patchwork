## ADDED Requirements

### Requirement: SimplifiedGameState exposes active-player query

`SimplifiedGameState` SHALL provide an `active_player()` const member function (or a free function `active_player(const SimplifiedGameState&)`) that returns 0 or 1 per the tie-breaking rule (lower position → active; tie → player 0).

#### Scenario: Active player query returns correct index

- **WHEN** player 0 has position 2 and player 1 has position 5
- **THEN** `active_player()` returns 0

#### Scenario: Active player query tie-breaks to player 0

- **WHEN** both players have position 10
- **THEN** `active_player()` returns 0

### Requirement: SimplifiedGameState tracks two leather-patch threshold flags

`SimplifiedGameState`'s shared state SHALL include two boolean flags: `leather_patch_26_claimed` and `leather_patch_53_claimed`, each defaulting to false. These SHALL be stored in the existing shared `uint64_t` word without changing the total struct size.

#### Scenario: Threshold flags default to false

- **WHEN** a `SimplifiedGameState` is default-constructed
- **THEN** both `leather_patch_26_claimed` and `leather_patch_53_claimed` are false

#### Scenario: Threshold-26 flag round-trips

- **WHEN** `leather_patch_26_claimed` is set to true
- **THEN** reading the flag returns true

#### Scenario: Threshold-53 flag round-trips

- **WHEN** `leather_patch_53_claimed` is set to true
- **THEN** reading the flag returns true

#### Scenario: Setting one flag does not affect the other

- **WHEN** `leather_patch_26_claimed` is set to true
- **THEN** `leather_patch_53_claimed` remains false
