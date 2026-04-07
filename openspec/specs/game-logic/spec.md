# Game Logic Specification

## Purpose
Defines the rules of the simplified Patchwork ruleset: which moves are legal, how a move transforms state, when the game ends, and how the score is computed. These are the verbs of the system.

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

`apply_move(state, Advance{})` SHALL advance the active player to the square exactly one ahead of the inactive player's position, credit 1 button per space advanced, and leave all other state unchanged. The circle marker and patch availability SHALL NOT change. Button income from any income spaces crossed during the advance is applied in addition (per the "Button income is paid when a player crosses a button income space" requirement).

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

### Requirement: Terminal detection identifies end of game

`is_terminal(state)` SHALL return true if and only if both players have a time-track position of 53 or greater. Position 53 represents "done" — the player has moved past the last active square on the time track.

#### Scenario: Game not terminal while a player is below 53

- **WHEN** player 0 has position 52 and player 1 has position 53
- **THEN** `is_terminal` returns false

#### Scenario: Game is terminal when both players are done

- **WHEN** both players have position ≥ 53
- **THEN** `is_terminal` returns true

### Requirement: Score computed as buttons minus twice free spaces plus bonus

`score(state, player_index)` SHALL return the integer value `buttons − 2 × free_spaces + bonus_points` for the given player, where `bonus_points` is 7 if that player holds the 7×7 bonus tile and 0 otherwise. `score` SHALL only be called on a terminal state; calling it on a non-terminal state is undefined behaviour.

#### Scenario: Score with no bonus and full quilt

- **WHEN** a terminal state has player 0 with 20 buttons and 0 free spaces, and the bonus is unclaimed
- **THEN** `score(state, 0)` returns 20

#### Scenario: Score with free spaces penalised

- **WHEN** a terminal state has player 0 with 10 buttons and 5 free spaces, and the bonus is unclaimed
- **THEN** `score(state, 0)` returns 0

#### Scenario: Bonus tile adds 7 to score

- **WHEN** a terminal state has player 0 with 10 buttons, 0 free spaces, and the 7×7 bonus
- **THEN** `score(state, 0)` returns 17

#### Scenario: Bonus tile not included for the other player

- **WHEN** player 0 holds the 7×7 bonus
- **THEN** `score(state, 1)` does not include the bonus 7 points

### Requirement: Winner is determined by score; equal scores resolved by first-to-finish

`winner(state)` SHALL return 0 if player 0's score is strictly greater than player 1's score, and 1 if player 1's score is strictly greater. When scores are equal, `winner` SHALL return the value of the `first_to_finish` field (0 or 1) — the player who first reached position ≥ 53 in that game. `winner` SHALL only be called on a terminal state. Both rulebooks state: "In case of a tie, the player who reached the final space of the time board first wins." This tiebreaker makes draws structurally impossible.

#### Scenario: Higher score wins

- **WHEN** player 0's score is 15 and player 1's score is 12
- **THEN** `winner` returns 0

#### Scenario: Equal scores resolved by first-to-finish

- **WHEN** both players have the same score and `first_to_finish` records player 1
- **THEN** `winner` returns 1
