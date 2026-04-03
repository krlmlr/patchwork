## ADDED Requirements

### Requirement: Terminal detection identifies end of game

`is_terminal(state)` SHALL return true if and only if both players have a time-track position of 54 or greater. Position 54 represents "done" — the player has moved past the last active square on the time track.

#### Scenario: Game not terminal while a player is below 54

- **WHEN** player 0 has position 53 and player 1 has position 54
- **THEN** `is_terminal` returns false

#### Scenario: Game is terminal when both players are done

- **WHEN** both players have position ≥ 54
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

### Requirement: Winner is the player with the higher score

`winner(state)` SHALL return 0 if player 0's score is strictly greater than player 1's score, 1 if player 1's score is strictly greater, and -1 for a draw. `winner` SHALL only be called on a terminal state. Draws are theoretically possible when both players achieve equal scores; no authoritative source confirms they are structurally impossible.

#### Scenario: Higher score wins

- **WHEN** player 0's score is 15 and player 1's score is 12
- **THEN** `winner` returns 0

#### Scenario: Draw returns -1

- **WHEN** both players have the same score
- **THEN** `winner` returns -1
