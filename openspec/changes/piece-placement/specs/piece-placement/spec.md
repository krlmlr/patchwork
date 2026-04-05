## ADDED Requirements

### Requirement: `orient_cells` produces correct rotations and flips

`orient_cells(patch, orientation_idx)` SHALL apply the specified rotation (0°/90°/180°/270°) and
optional horizontal flip to the patch's cell list, then normalise the result so that the minimum
row and minimum column are both 0.

#### Scenario: Identity orientation (index 0) matches original cells

- **WHEN** `orient_cells` is called with orientation index 0 on any patch
- **THEN** the returned cells are the normalised form of the patch's original cell list

#### Scenario: 90° rotation is applied correctly

- **WHEN** `orient_cells` is called with the 90°-rotation index on a known asymmetric patch
- **THEN** the returned cells match the expected transformed and normalised result

#### Scenario: Horizontal flip is applied correctly

- **WHEN** `orient_cells` is called with the flip index on a known asymmetric patch
- **THEN** the returned cell set is a horizontal mirror of the identity orientation

#### Scenario: All 8 candidates are generated

- **WHEN** all 8 orientation indices are applied to an asymmetric patch
- **THEN** exactly 8 distinct normalised cell lists are produced

### Requirement: `unique_orientations` de-duplicates symmetric patches

`unique_orientations(patch)` SHALL return only the distinct orientations of a patch, removing
duplicates caused by rotational or reflective symmetry.

#### Scenario: Symmetric patch has fewer than 8 distinct orientations

- **WHEN** `unique_orientations` is called on a patch with 2-fold rotational symmetry (e.g., a
  straight tetromino or 2×2 square)
- **THEN** the returned list has fewer than 8 elements

#### Scenario: Asymmetric patch has exactly 8 distinct orientations

- **WHEN** `unique_orientations` is called on a patch with no symmetry
- **THEN** the returned list has exactly 8 elements

### Requirement: `fits_at` validates patch placement on the quilt board

`fits_at(board, cells, row, col)` SHALL return `true` if and only if every cell in `cells`, when
offset by the anchor `(row, col)`, is within the 9×9 grid and corresponds to an unoccupied cell on
the board.

#### Scenario: Patch fits on empty board

- **GIVEN** an empty `PlayerState` board
- **WHEN** `fits_at` is called with a valid anchor where all cells are in-bounds
- **THEN** the result is `true`

#### Scenario: Patch overlaps occupied cell

- **GIVEN** a `PlayerState` with at least one occupied cell
- **WHEN** `fits_at` is called with an anchor that would place a patch cell on the occupied cell
- **THEN** the result is `false`

#### Scenario: Patch extends out of bounds

- **WHEN** `fits_at` is called with an anchor such that at least one patch cell maps outside the
  9×9 grid
- **THEN** the result is `false`

### Requirement: `visible_patches` returns the three patches ahead of the circle marker

`visible_patches(setup, state)` SHALL return a `std::array<int, 3>` of patch IDs corresponding to
positions `(circle_marker + 1) % 33`, `(circle_marker + 2) % 33`, and `(circle_marker + 3) % 33`
in `setup.circle`.

#### Scenario: Visible patches match circle positions

- **GIVEN** a `GameSetup` with a known circle arrangement and a `GameState` with `circle_marker`
  set to some value `m`
- **WHEN** `visible_patches(setup, state)` is called
- **THEN** the returned IDs equal `circle[(m+1)%33]`, `circle[(m+2)%33]`, `circle[(m+3)%33]`

#### Scenario: Wrapping at the end of the circle

- **GIVEN** a `GameState` with `circle_marker` set to 31 (near the end)
- **WHEN** `visible_patches(setup, state)` is called
- **THEN** the returned IDs include patches from positions 32, 0, and 1 (wrapping around)

### Requirement: `legal_moves` enumerates all legal moves

`legal_moves(setup, state, player_idx)` SHALL return a `std::vector<Move>` containing:

1. One `AdvanceAndReceive` move (always legal as long as the player's position is less than 53).
2. For each of the three visible patches the current player can afford (buttons ≥ patch button
   cost), one `PlacePatch` move per (orientation, anchor) combination where `fits_at` returns
   `true`.

#### Scenario: `AdvanceAndReceive` is always included when not at end

- **GIVEN** a game state where the current player's position is less than 53
- **WHEN** `legal_moves` is called
- **THEN** the result contains exactly one `AdvanceAndReceive` move

#### Scenario: Affordable patches with valid placements are included

- **GIVEN** a game state where the current player has enough buttons to buy at least one visible
  patch and there is at least one valid placement on the board
- **WHEN** `legal_moves` is called
- **THEN** the result contains at least one `PlacePatch` move for that patch

#### Scenario: Unaffordable patches are excluded

- **GIVEN** a game state where the current player has 0 buttons
- **WHEN** `legal_moves` is called
- **THEN** the result contains no `PlacePatch` moves

#### Scenario: No valid placements excluded

- **GIVEN** a game state where the quilt board is full and no patch can be placed
- **WHEN** `legal_moves` is called
- **THEN** the result contains only the `AdvanceAndReceive` move

### Requirement: `apply` updates `GameState` correctly for `PlacePatch`

`apply(setup, state, player_idx, move)` with a `PlacePatch` move SHALL:

1. Deduct the patch's button cost from the player's button balance.
2. Add the patch's income value to the player's income.
3. Advance the player's time-track position by the patch's time cost (capped at 53).
4. Award button income (`player.income × N`) for each button-income space crossed during the
   advance (where N is the count of income spaces strictly between old and new positions).
5. Stamp each oriented+anchored cell as occupied on the player's board.
6. Mark the chosen patch as unavailable in `GameState`.
7. Advance the circle marker by `circle_offset + 1` (modulo 33).

#### Scenario: Buttons are deducted correctly

- **GIVEN** a player with 10 buttons who places a patch costing 3 buttons
- **WHEN** `apply` is called with the corresponding `PlacePatch` move
- **THEN** the player's button balance is 7

#### Scenario: Income is updated correctly

- **GIVEN** a player with income 2 who places a patch with income 3
- **WHEN** `apply` is called
- **THEN** the player's income is 5

#### Scenario: Time advances by patch time cost

- **GIVEN** a player at position 10 who places a patch with time cost 4
- **WHEN** `apply` is called
- **THEN** the player's position is 14

#### Scenario: Button income is awarded when crossing income spaces

- **GIVEN** a player at position 3 with income 2 who places a patch with time cost 4
  (so they move from 3 to 7, crossing the income space at position 5)
- **WHEN** `apply` is called
- **THEN** the player receives 2 additional buttons (income × 1 crossed space)

#### Scenario: Board cells are stamped

- **GIVEN** an empty board and a `PlacePatch` move with a known orientation and anchor
- **WHEN** `apply` is called
- **THEN** exactly the expected cells on the board are occupied

#### Scenario: Patch is marked unavailable

- **WHEN** `apply` is called with a `PlacePatch` move for patch ID `p`
- **THEN** `state.patch_available(p)` returns `false`

#### Scenario: Circle marker advances past chosen patch

- **GIVEN** a `GameState` with `circle_marker` at position `m` and a `PlacePatch` move with
  `circle_offset` = 2
- **WHEN** `apply` is called
- **THEN** `state.circle_marker()` equals `(m + 3) % 33`

### Requirement: `apply` updates `GameState` correctly for `AdvanceAndReceive`

`apply(setup, state, player_idx, move)` with an `AdvanceAndReceive` move SHALL:

1. Advance the player's position to `other_player.position() + 1` (capped at 53).
2. Award `spaces_moved` buttons (one per space advanced).
3. Award button income for each income space crossed.

#### Scenario: Position advances to one ahead of other player

- **GIVEN** player 0 at position 5 and player 1 at position 10
- **WHEN** `apply` is called with `AdvanceAndReceive` for player 0
- **THEN** player 0's position is 11

#### Scenario: Buttons received equal spaces moved

- **GIVEN** player 0 at position 5 and player 1 at position 10
- **WHEN** `apply` is called with `AdvanceAndReceive` for player 0
- **THEN** player 0's button balance increases by 6 (moved 6 spaces: 6 to 11)

#### Scenario: Button income awarded when crossing income spaces

- **GIVEN** player 0 at position 4 with income 3 and player 1 at position 12
  (player 0 moves from 4 to 13, crossing income spaces at 5 and 11)
- **WHEN** `apply` is called with `AdvanceAndReceive` for player 0
- **THEN** player 0 receives 9 × 1 (spaces) + 3 × 2 (income × 2 crossed) = 15 additional buttons

### Requirement: Button income spaces are the 9 canonical positions

The 9 button-income space positions on the time track SHALL be encoded as a `constexpr` array and
SHALL equal `{5, 11, 17, 23, 29, 35, 41, 47, 53}`.

#### Scenario: Income space array has exactly 9 entries

- **WHEN** `kIncomeSpaces` is inspected at compile time
- **THEN** it has exactly 9 elements

#### Scenario: Income space values match Patchwork rules

- **WHEN** `kIncomeSpaces` is inspected
- **THEN** the values are `{5, 11, 17, 23, 29, 35, 41, 47, 53}` in order

### Requirement: `current_player` returns the player with the lower position

`current_player(const GameState&) → int` SHALL return 0 if `player(0).position() ≤
player(1).position()`, and 1 otherwise.

#### Scenario: Player with lower position is returned

- **GIVEN** a `GameState` where player 0 is at position 5 and player 1 is at position 10
- **WHEN** `current_player(state)` is called
- **THEN** the result is 0

#### Scenario: Ties return player 0

- **GIVEN** a `GameState` where both players are at the same position
- **WHEN** `current_player(state)` is called
- **THEN** the result is 0

### Requirement: Piece-placement logic is unit tested

All piece-placement behaviours SHALL have Catch2 unit tests covering the orientation engine, fit
checker, visible patches, legal move generation, and move application.

#### Scenario: Tests exist and pass

- **WHEN** `meson test -C build` is run
- **THEN** all piece-placement tests pass with exit code 0
