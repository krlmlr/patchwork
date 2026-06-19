## MODIFIED Requirements

### Requirement: Game-end event is logged when the game reaches a terminal state

The logger SHALL write a JSON object line of type `"game_end"` after the last move is applied and the terminal state is detected. The event SHALL include: `event` (`"game_end"`), `score_p0` and `score_p1` (integer scores), and `winner` (`0` or `1`). Draws are structurally impossible: the `first_to_finish` tiebreaker resolves equal scores (see the game-logic `winner` requirement), so `winner` SHALL be `0` or `1` only and SHALL NOT take any other value such as `-1`.

#### Scenario: Game-end line records correct scores and winner

- **WHEN** the game ends with player 0 score 18 and player 1 score 14
- **THEN** the game-end line has `"score_p0": 18`, `"score_p1": 14`, `"winner": 0`

#### Scenario: Equal scores are resolved by first-to-finish, never a draw

- **WHEN** both players finish with equal scores and `first_to_finish` records player 1
- **THEN** the game-end line has `"winner": 1`
- **AND** `winner` is never `-1`
