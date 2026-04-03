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

`apply_move(state, Advance{})` SHALL advance the active player to the square exactly one ahead of the inactive player's position, credit buttons equal to the active player's income, and leave all other state unchanged. The circle marker and patch availability SHALL NOT change.

#### Scenario: Position advances to one ahead of opponent

- **WHEN** active player is at position 3 and inactive player is at position 8
- **THEN** the active player's position in the successor state equals 9

#### Scenario: Buttons are credited from income

- **WHEN** active player has income 4 and 6 buttons, and advances
- **THEN** the active player's buttons in the successor state equals 10

#### Scenario: Circle marker unchanged after advance

- **WHEN** an Advance move is applied
- **THEN** the circle marker is the same in the predecessor and successor states

### Requirement: Button income is paid when a player crosses a button income space

The time track has button income spaces at positions 5, 11, 17, 23, 29, 35, 41, and 47 (every 6 squares from 5). When a player's position crosses any of these squares (old position < threshold ≤ new position) as a result of any move, that player SHALL receive buttons equal to their current income.

#### Scenario: Income paid on crossing an income space while buying a patch

- **WHEN** a player at position 4 buys a patch with time cost 2 (moving to position 6)
- **THEN** the player's buttons are increased by their income in addition to any other button changes from the patch

#### Scenario: Income paid on crossing an income space while advancing

- **WHEN** a player at position 10 advances to position 12
- **THEN** the player's buttons are increased by their income

#### Scenario: Multiple income spaces crossed in one move

- **WHEN** a player at position 4 buys a patch with time cost 14 (moving to position 18)
- **THEN** the player's buttons are increased by 2 × income (crossing positions 5 and 11)

### Requirement: Leather patches are awarded at time thresholds

The time track has two 1×1 leather patch squares at positions 26 and 53. The first player to reach or pass position 26 SHALL receive a leather patch (free_spaces decremented by 1, threshold marked claimed). The first player to reach or pass position 53 SHALL likewise receive a leather patch. If both players cross a threshold in the same move (impossible in a two-player turn-based game) the active player receives the patch.

#### Scenario: Leather patch awarded at threshold 26

- **WHEN** a player's position crosses 26 and the threshold-26 leather patch has not been claimed
- **THEN** that player's free_spaces is decremented by 1 and the threshold-26 flag is set

#### Scenario: Leather patch not awarded again at same threshold

- **WHEN** the threshold-26 leather patch has already been claimed and a second player crosses position 26
- **THEN** the second player does NOT receive a leather patch

#### Scenario: Leather patch awarded at threshold 53

- **WHEN** a player's position reaches 53 and the threshold-53 leather patch has not been claimed
- **THEN** that player's free_spaces is decremented by 1 and the threshold-53 flag is set

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
