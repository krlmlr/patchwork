## ADDED Requirements

### Requirement: Move type represents all legal actions

A `Move` type SHALL represent exactly two variants: `BuyPatch` (buying a patch by its index in `kPatches`) and `Advance` (advancing past the leading player and earning buttons). No other move variants are valid in the simplified ruleset.

#### Scenario: BuyPatch variant carries patch index

- **WHEN** a `Move` is constructed as `BuyPatch` with a patch index in 0–32
- **THEN** the move holds that index and can be retrieved without loss

#### Scenario: Advance variant is distinct from BuyPatch

- **WHEN** two moves are constructed — one `Advance` and one `BuyPatch{0}`
- **THEN** they compare as not equal

### Requirement: Active player is determined by time-track position

`active_player(state)` SHALL return the index (0 or 1) of the player whose time-track position is strictly less than the other player's. When both positions are equal, player 0 SHALL be returned.

#### Scenario: Player with lower position is active

- **WHEN** player 0 has position 3 and player 1 has position 7
- **THEN** `active_player` returns 0

#### Scenario: Player with higher position is not active

- **WHEN** player 0 has position 10 and player 1 has position 4
- **THEN** `active_player` returns 1

#### Scenario: Tie is broken in favour of player 0

- **WHEN** both players have the same time-track position
- **THEN** `active_player` returns 0

### Requirement: Legal moves include up to three patches ahead of the circle marker

`legal_moves(state)` SHALL include a `BuyPatch` move for each of the up to three patches that are available (not taken) immediately ahead of the circle marker in circular order. Patches beyond the third available patch SHALL NOT be included.

#### Scenario: Three patches available ahead of marker

- **WHEN** the three patches immediately ahead of the circle marker are all available
- **THEN** `legal_moves` includes exactly three `BuyPatch` moves plus the `Advance` move

#### Scenario: Fewer than three patches remain

- **WHEN** only one patch is available in the entire circle
- **THEN** `legal_moves` includes exactly one `BuyPatch` move plus the `Advance` move

#### Scenario: Active player cannot afford any patch

- **WHEN** the active player's button balance is less than the cost of every visible patch
- **THEN** `legal_moves` includes only the `Advance` move (no `BuyPatch` moves)

#### Scenario: Active player cannot reach any patch in time

- **WHEN** buying any visible patch would advance the player's position beyond 53
- **THEN** those patches are excluded from `legal_moves`

### Requirement: Advance move is always legal

`legal_moves(state)` SHALL always include the `Advance` move unless the game is already terminal.

#### Scenario: Advance present when patches exist

- **WHEN** the game is not terminal and patches are available
- **THEN** `legal_moves` includes the `Advance` move

#### Scenario: Advance present when no patches are affordable

- **WHEN** the active player has 0 buttons and all visible patches cost at least 1 button
- **THEN** `legal_moves` still includes the `Advance` move

### Requirement: Legal moves returns empty list for a terminal state

`legal_moves(state)` SHALL return an empty collection when the game is terminal (both players at position ≥ 53).

#### Scenario: No moves when game is over

- **WHEN** both players have position 53
- **THEN** `legal_moves` returns an empty collection
