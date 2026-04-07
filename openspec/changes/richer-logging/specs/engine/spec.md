## MODIFIED Requirements

### Requirement: Game-start event is logged when a game begins

The logger SHALL write a JSON object line (NDJSON) of type `"game_start"` immediately before the first move is applied. The event SHALL include: `event` (`"game_start"`), `seed` (integer), `setup_id` (integer or string), initial state summary (both players' starting buttons, income, and free_spaces), and `circle` — a 33-character string of single-character patch names in circle order from the `GameSetup`, encoding the full initial patch arrangement.

#### Scenario: Game-start line is well-formed JSON

- **WHEN** a game begins and the logger is attached
- **THEN** the first line written is valid JSON with `"event": "game_start"`

#### Scenario: Game-start records seed and setup

- **WHEN** a game is started with seed 42 and setup id 7
- **THEN** the game-start line includes `"seed": 42` and `"setup_id": 7`

#### Scenario: Game-start includes 33-character circle string

- **WHEN** `log_game_start` is called with a valid `GameSetup`
- **THEN** the emitted JSON line contains a `"circle"` field whose value is a 33-character string of patch-name characters in `GameSetup` circle order

#### Scenario: Circle string encodes patch names not IDs

- **WHEN** the circle is serialised at game start
- **THEN** each character SHALL be the single-character patch name (as stored in `kPatches[id].name`), not the numeric patch ID

### Requirement: Move event is logged for each move applied

The logger SHALL write a JSON object line of type `"move"` after each move is applied. The event SHALL include: `event` (`"move"`), `ply` (0-based move counter), `player` (0 or 1), `move_type` (`"buy_patch"` or `"advance"`), and for buy-patch moves the `patch_index`. The active player's post-move `position`, `buttons`, `income`, `free_spaces`, and `board_value` SHALL be recorded. A `circle` field SHALL contain the currently available patches in circle order starting from the current marker position as a string of patch-name characters.

#### Scenario: Move line contains expected fields for BuyPatch

- **WHEN** player 0 buys patch index 5
- **THEN** the logged line has `"event": "move"`, `"player": 0`, `"move_type": "buy_patch"`, `"patch_index": 5`

#### Scenario: Move line contains expected fields for Advance

- **WHEN** player 1 advances
- **THEN** the logged line has `"event": "move"`, `"player": 1`, `"move_type": "advance"`

#### Scenario: Ply increments with each move

- **WHEN** five moves are applied in sequence
- **THEN** the five move lines have `"ply"` values 0, 1, 2, 3, 4

#### Scenario: Move line includes income and free_spaces

- **WHEN** `log_move` is called after any move
- **THEN** the emitted JSON line contains `"income"` and `"free_spaces"` matching the active player's post-move state

#### Scenario: Move line includes board_value proxy

- **WHEN** a player holds exactly 5 buttons (starting amount, no income yet)
- **THEN** `"board_value"` SHALL be 0

#### Scenario: Move circle shrinks after buy

- **WHEN** `log_move` is called after a `buy_patch` move
- **THEN** the `"circle"` field SHALL NOT contain the purchased patch's name character

#### Scenario: Move circle wraps from marker

- **WHEN** the `circle_marker()` is non-zero
- **THEN** the `"circle"` string SHALL start from the patch at the marker position and wrap around, omitting unavailable patches

### Requirement: Game-end event is logged when the game reaches a terminal state

The logger SHALL write a JSON object line of type `"game_end"` after the last move is applied and the terminal state is detected. The event SHALL include: `event` (`"game_end"`), `score_p0` and `score_p1` (integer scores), `winner` (0, 1, or -1 for draw), and `p0`/`p1` objects each containing final `income` and `free_spaces` for both players.

#### Scenario: Game-end line records correct scores and winner

- **WHEN** the game ends with player 0 score 18 and player 1 score 14
- **THEN** the game-end line has `"score_p0": 18`, `"score_p1": 14`, `"winner": 0`

#### Scenario: Game-end records draw

- **WHEN** both players finish with equal scores
- **THEN** the game-end line has `"winner": -1`

#### Scenario: Game-end includes per-player income and free_spaces

- **WHEN** `log_game_end` is called
- **THEN** the emitted JSON line contains `"p0"` and `"p1"` objects each with `"income"` and `"free_spaces"` matching the final state

### Requirement: Log output is NDJSON (one JSON object per line)

Each log line SHALL be a self-contained valid JSON object terminated by a newline. There SHALL be no trailing commas, no surrounding array brackets, and no blank lines between events.

#### Scenario: Each line is independently parseable

- **WHEN** a complete game log is produced
- **THEN** every line, read individually, parses as a valid JSON object
