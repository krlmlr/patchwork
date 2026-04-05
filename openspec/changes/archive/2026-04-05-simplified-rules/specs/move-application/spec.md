## ADDED Requirements

### Requirement: BuyPatch move updates buyer's state correctly

`apply_move(state, BuyPatch{i})` SHALL return a state where the active player's buttons are reduced by the patch's button cost, position is advanced by the patch's time cost, income is increased by the patch's income value, and free_spaces is decreased by the patch's num_cells. The patch SHALL be marked unavailable in the shared state and the circle marker SHALL be updated to the position just after the purchased patch.

#### Scenario: Button cost is deducted

- **WHEN** a patch with button cost 3 is purchased by a player with 10 buttons
- **THEN** the player's buttons in the successor state equal 7

#### Scenario: Time cost advances position

- **WHEN** a patch with time cost 4 is purchased by a player at position 5
- **THEN** the player's position in the successor state equals 9

#### Scenario: Income is added to player's income field

- **WHEN** a patch with income 2 is purchased by a player with income 1
- **THEN** the player's income in the successor state equals 3

#### Scenario: Free spaces are reduced by patch cell count

- **WHEN** a patch with 5 cells is purchased by a player with 81 free spaces
- **THEN** the player's free_spaces in the successor state equals 76

#### Scenario: Patch is removed from availability

- **WHEN** patch index i is purchased
- **THEN** `patch_available(i)` returns false in the successor state

#### Scenario: Circle marker advances past the purchased patch

- **WHEN** patch at position k in circular order is purchased and the marker was at 0
- **THEN** the circle marker in the successor state points to the position just after k

### Requirement: Advance move updates active player's state correctly

`apply_move(state, Advance{})` SHALL advance the active player to the square exactly one ahead of the inactive player's position, credit 1 button per space advanced, and leave all other state unchanged. The circle marker and patch availability SHALL NOT change. Button income from any income spaces crossed during the advance is applied in addition (per the "Button income is paid when crossing an income space" requirement).

#### Scenario: Position advances to one ahead of opponent

- **WHEN** active player is at position 3 and inactive player is at position 8
- **THEN** the active player's position in the successor state equals 9

#### Scenario: Buttons credited at 1 per space advanced

- **WHEN** active player at position 3 advances to position 9 (6 spaces)
- **THEN** the active player's buttons in the successor state are increased by 6

#### Scenario: Circle marker unchanged after advance

- **WHEN** an Advance move is applied
- **THEN** the circle marker is the same in the predecessor and successor states

### Requirement: Button income is paid when a player crosses a button income space

The time track has button income spaces at positions 5, 11, 17, 23, 29, 35, 41, 47, and 53 (every 6 squares from 5, inclusive of 53). When a player's position crosses any of these squares (old position < threshold ≤ new position) as a result of any move, that player SHALL receive buttons equal to their current income.

#### Scenario: Income paid on crossing an income space while buying a patch

- **WHEN** a player at position 4 buys a patch with time cost 2 (moving to position 6)
- **THEN** the player's buttons are increased by their income in addition to any other button changes from the patch

#### Scenario: Income paid on crossing an income space while advancing

- **WHEN** a player at position 10 advances to position 12
- **THEN** the player's buttons are increased by their income

#### Scenario: Multiple income spaces crossed in one move

- **WHEN** a player at position 4 buys a patch with time cost 14 (moving to position 18)
- **THEN** the player's buttons are increased by 2 × income (crossing positions 5 and 11)

#### Scenario: Income paid on crossing position 53

- **WHEN** a player at position 50 advances to position 53
- **THEN** the player's buttons are increased by their income (crossing position 53)

### Requirement: Leather patches are awarded when a player is the first to cross each of five time thresholds

The time track has five 1×1 leather patch squares at positions 26, 32, 38, 44, and 50. When the active player's new position first crosses any of these thresholds (old position < threshold ≤ new position), and neither player's pre-move position was already ≥ that threshold, the active player SHALL receive a leather patch: `free_spaces` is decremented by 1. Placement is mandatory. No new state field is required: whether a threshold has been claimed is derived from the players' pre-move positions — if either player was already at or past the threshold before this move, the patch is already taken.

#### Scenario: Leather patch awarded at first crossing of threshold 26

- **WHEN** a player's position crosses 26 and neither player's pre-move position was ≥ 26
- **THEN** that player's free_spaces is decremented by 1

#### Scenario: Leather patch not awarded when threshold already passed by either player

- **WHEN** the other player's position is already ≥ 26 and the active player crosses 26
- **THEN** the active player does NOT receive a leather patch for threshold 26

#### Scenario: All five leather thresholds are checked independently

- **WHEN** a player at position 25 buys a patch that moves them to position 39
- **THEN** the player's free_spaces is decremented by 2 (crossing thresholds 26 and 32, assuming neither was yet claimed)

### Requirement: 7×7 bonus is claimed when a player reaches 56 occupied cells

The 7×7 bonus tile is claimed by the first player whose occupied cells (81 − free_spaces) reach or exceed 56. After each `BuyPatch` move, if the bonus is unclaimed and the buyer's occupied cells ≥ 56, the bonus status SHALL be set to that player.

#### Scenario: Bonus claimed on reaching 56 occupied cells

- **WHEN** a player purchases a patch that brings their occupied cells from 55 to 60 and the bonus is unclaimed
- **THEN** the bonus status is set to that player

#### Scenario: Bonus not re-awarded

- **WHEN** a player already holds the bonus and a second player reaches 56+ occupied cells
- **THEN** the bonus status remains with the first player

#### Scenario: Advance move does not affect bonus

- **WHEN** an Advance move is applied regardless of occupied cells
- **THEN** the bonus status is unchanged

### Requirement: Non-active player's state is unchanged by any move

`apply_move` SHALL leave the inactive player's `SimplifiedPlayerState` identical in the successor state.

#### Scenario: Inactive player unchanged after BuyPatch

- **WHEN** player 0 buys a patch
- **THEN** player 1's free_spaces, position, buttons, and income are identical in the predecessor and successor states

#### Scenario: Inactive player unchanged after Advance

- **WHEN** player 0 advances
- **THEN** player 1's free_spaces, position, buttons, and income are identical in the predecessor and successor states
