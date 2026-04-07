# move-generation Specification

## Purpose
Defines the `Move` type and legal move generation for the simplified Patchwork ruleset, covering BuyPatch and Advance variants.

## Requirements

### Requirement: Move type represents all legal actions

A `Move` type SHALL represent exactly two variants: `BuyPatch` (buying a patch by its index in `kPatches`) and `Advance` (advancing past the leading player and earning 1 button per space moved). No other move variants are valid in the simplified ruleset.

#### Scenario: BuyPatch variant carries patch index

- **WHEN** a `Move` is constructed as `BuyPatch` with a patch index in 0–32
- **THEN** the move holds that index and can be retrieved without loss

#### Scenario: Advance variant is distinct from BuyPatch

- **WHEN** two moves are constructed — one `Advance` and one `BuyPatch{0}`
- **THEN** they compare as not equal

### Requirement: Active player is determined by a tracked `next_player` field

`active_player(state)` SHALL return the value of the `next_player` field stored in `SimplifiedGameState`. The active player changes only when the moved player's new position strictly exceeds the inactive player's position, at which point the other player becomes `next_player`. When positions are equal after a move, the moved player remains `next_player` (they have not yet overtaken the opponent). At game start, player 0 is `next_player`.

#### Scenario: Player with lower position is active

- **WHEN** player 0 has position 3 and player 1 has position 7 and next_player is 0
- **THEN** `active_player` returns 0

#### Scenario: Player with higher position is not active

- **WHEN** next_player field records player 1 as active
- **THEN** `active_player` returns 1

#### Scenario: Active player remains active when landing on opponent's square

- **WHEN** player 0 (active) buys a patch that moves them from position 3 to position 8, and player 1 is at position 8
- **THEN** player 0 remains the active player (has not overtaken player 1)

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

- **WHEN** the active player's position plus any visible patch's time cost would still be a valid position (no cap; all patches are reachable in time)
- **THEN** all affordable visible patches are included in `legal_moves` regardless of resulting position

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

- **WHEN** both players have position ≥ 53
- **THEN** `legal_moves` returns an empty collection
