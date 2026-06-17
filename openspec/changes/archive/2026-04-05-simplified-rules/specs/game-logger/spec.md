## ADDED Requirements

### Requirement: Game-start event is logged when a game begins

The logger SHALL write a JSON object line (NDJSON) of type `"game_start"` immediately before the first move is applied. The event SHALL include: `event` (`"game_start"`), `seed` (integer), `setup_id` (integer or string), and initial state summary (both players' starting buttons, income, and free_spaces).

#### Scenario: Game-start line is well-formed JSON

- **WHEN** a game begins and the logger is attached
- **THEN** the first line written is valid JSON with `"event": "game_start"`

#### Scenario: Game-start records seed and setup

- **WHEN** a game is started with seed 42 and setup id 7
- **THEN** the game-start line includes `"seed": 42` and `"setup_id": 7`

### Requirement: Move event is logged for each move applied

The logger SHALL write a JSON object line of type `"move"` after each move is applied. The event SHALL include: `event` (`"move"`), `ply` (0-based move counter), `player` (0 or 1), `move_type` (`"buy_patch"` or `"advance"`), and for buy-patch moves the `patch_index`. The new active player's position and buttons SHALL also be recorded.

#### Scenario: Move line contains expected fields for BuyPatch

- **WHEN** player 0 buys patch index 5
- **THEN** the logged line has `"event": "move"`, `"player": 0`, `"move_type": "buy_patch"`, `"patch_index": 5`

#### Scenario: Move line contains expected fields for Advance

- **WHEN** player 1 advances
- **THEN** the logged line has `"event": "move"`, `"player": 1`, `"move_type": "advance"`

#### Scenario: Ply increments with each move

- **WHEN** five moves are applied in sequence
- **THEN** the five move lines have `"ply"` values 0, 1, 2, 3, 4

### Requirement: Game-end event is logged when the game reaches a terminal state

The logger SHALL write a JSON object line of type `"game_end"` after the last move is applied and the terminal state is detected. The event SHALL include: `event` (`"game_end"`), `score_p0` and `score_p1` (integer scores), and `winner` (0, 1, or -1 for draw).

#### Scenario: Game-end line records correct scores and winner

- **WHEN** the game ends with player 0 score 18 and player 1 score 14
- **THEN** the game-end line has `"score_p0": 18`, `"score_p1": 14`, `"winner": 0`

#### Scenario: Game-end records draw

- **WHEN** both players finish with equal scores
- **THEN** the game-end line has `"winner": -1`

### Requirement: Log output is NDJSON (one JSON object per line)

Each log line SHALL be a self-contained valid JSON object terminated by a newline. There SHALL be no trailing commas, no surrounding array brackets, and no blank lines between events.

#### Scenario: Each line is independently parseable

- **WHEN** a complete game log is produced
- **THEN** every line, read individually, parses as a valid JSON object
