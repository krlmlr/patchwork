# Engine Specification

## Purpose
Defines the game loop and everything that makes a game run reproducibly end-to-end: the play driver executable, NDJSON event logging, and seed/setup plumbing.
## Requirements
### Requirement: Play driver runs a complete game between two random agents and writes a log

The play driver executable SHALL accept `--seed <n>` and `--setup <id>` command-line arguments, run a full game between two random agents on the given setup starting with the given seed, and write the NDJSON game log to stdout. An optional `--output <file>` argument SHALL redirect the log to a file instead.

#### Scenario: Game terminates and log is written

- **WHEN** the play driver is invoked with valid `--seed` and `--setup` arguments
- **THEN** it exits with code 0 and the output contains exactly one `game_start` line, one or more `move` lines, and exactly one `game_end` line

#### Scenario: Same seed and setup produce identical log

- **WHEN** the play driver is invoked twice with the same `--seed` and `--setup` values
- **THEN** the two output logs are byte-for-byte identical

#### Scenario: Output redirected to file with --output

- **WHEN** `--output game.ndjson` is given
- **THEN** the log is written to `game.ndjson` and nothing is written to stdout

### Requirement: Play driver reports an error for invalid arguments
The play driver SHALL print a usage message to stderr and exit with a non-zero code when `--seed` or `--setup` is missing or non-numeric.

- **WHEN** `--seed` or `--setup` is missing or non-numeric
- **THEN** the play driver prints a usage message to stderr and exits with a non-zero code

#### Scenario: Missing seed argument

- **WHEN** the play driver is invoked without `--seed`
- **THEN** it exits with a non-zero code and prints an error to stderr

### Requirement: Game-start event is logged when a game begins

The logger SHALL write a JSON object line (NDJSON) of type `"game_start"` immediately before the first move is applied. The event SHALL include: `event` (`"game_start"`), `seed` (integer), `setup_id` (integer or string), and an initial state summary with both players' starting `buttons`, `income`, and `free_spaces`. It SHALL also include a `circle` field — a 33-character string of single-character patch names in circle order from the `GameSetup`. By game convention the `'2'` tile (the two-square patch) is always the last character of the circle string, as the neutral token is placed immediately after it.

#### Scenario: Game-start line is well-formed JSON

- **WHEN** a game begins and the logger is attached
- **THEN** the first line written is valid JSON with `"event": "game_start"`

#### Scenario: Game-start records seed and setup

- **WHEN** a game is started with seed 42 and setup id 7
- **THEN** the game-start line includes `"seed": 42` and `"setup_id": 7`

#### Scenario: Game-start includes 33-character circle string

- **WHEN** `log_game_start` is called with a valid `GameSetup`
- **THEN** the emitted JSON line contains a `"circle"` field whose value is a 33-character string of patch-name characters in `GameSetup` circle order

#### Scenario: Circle string ends with the two-square patch '2'

- **WHEN** the circle is serialised at game start
- **THEN** the last character of the circle string SHALL be `'2'` (the two-square patch, by neutral-token convention)

#### Scenario: Circle string encodes patch names not IDs

- **WHEN** the circle is serialised at game start
- **THEN** each character SHALL be the single-character patch name (as stored in `kPatches[id].name`), not the numeric patch ID

### Requirement: Move event is logged for each move applied

The logger SHALL write a JSON object line of type `"move"` after each move is applied. The event SHALL include: `event` (`"move"`), `ply` (0-based move counter), `player` (0 or 1), `move_type` (`"buy_patch"` or `"advance"`), and for buy-patch moves the `patch_index` and `patch_symbol` (single-character patch name). The active player's post-move `position`, `buttons`, `income`, `free_spaces`, `board_value`, `projected_income`, and `projected_score` SHALL be recorded (see glossary for definitions). A `circle` field SHALL contain the currently available patches in circle order starting from the current marker position as a string of patch-name characters.

#### Scenario: Move line contains expected fields for BuyPatch

- **WHEN** player 0 buys patch index 5
- **THEN** the logged line has `"event": "move"`, `"player": 0`, `"move_type": "buy_patch"`, `"patch_index": 5`, and `"patch_symbol": "4"` (the name character of kPatches[5])

#### Scenario: Move line contains expected fields for Advance

- **WHEN** player 1 advances
- **THEN** the logged line has `"event": "move"`, `"player": 1`, `"move_type": "advance"`

#### Scenario: Ply increments with each move

- **WHEN** five moves are applied in sequence
- **THEN** the five move lines have `"ply"` values 0, 1, 2, 3, 4

#### Scenario: Move line includes income and free_spaces

- **WHEN** `log_move` is called after any move
- **THEN** the emitted JSON line contains `"income"` and `"free_spaces"` matching the active player's post-move state

#### Scenario: Move line includes board_value as game-score formula

- **WHEN** a player holds exactly 5 buttons and 81 free spaces (starting state)
- **THEN** `"board_value"` SHALL be `5 - 2*81 = -157`

#### Scenario: Move line includes projected_income and projected_score

- **WHEN** `log_move` is called after any move
- **THEN** the emitted JSON line contains `"projected_income"` and `"projected_score"` fields
- **AND** `projected_income` equals `income × (number of income spaces ahead of the player's position)`
- **AND** `projected_score` equals `buttons + projected_income - 2 × free_spaces`

#### Scenario: projected_income is zero at or past the last income space

- **WHEN** a player's position is ≥ 53 (last income space)
- **THEN** `"projected_income"` SHALL be 0 regardless of the player's income rate

#### Scenario: Move circle shrinks after buy

- **WHEN** `log_move` is called after a `buy_patch` move
- **THEN** the `"circle"` field SHALL NOT contain the purchased patch's name character

#### Scenario: Move circle wraps from marker

- **WHEN** the `circle_marker()` is non-zero
- **THEN** the `"circle"` string SHALL start from the patch at the marker position and wrap around, omitting unavailable patches

### Requirement: Game-end event is logged when the game reaches a terminal state

The logger SHALL write a JSON object line of type `"game_end"` after the last move is applied and the terminal state is detected. The event SHALL include: `event` (`"game_end"`), `score_p0` and `score_p1` (integer scores), `winner` (`0` or `1`), and `p0`/`p1` objects each containing final `income` and `free_spaces` for both players. Draws are structurally impossible: the `first_to_finish` tiebreaker resolves equal scores, so `winner` SHALL be `0` or `1` only and SHALL NOT take any other value such as `-1`.

#### Scenario: Game-end line records correct scores and winner

- **WHEN** the game ends with player 0 score 18 and player 1 score 14
- **THEN** the game-end line has `"score_p0": 18`, `"score_p1": 14`, `"winner": 0`

#### Scenario: Equal scores are resolved by first-to-finish, never a draw

- **WHEN** both players finish with equal scores and `first_to_finish` records player 1
- **THEN** the game-end line has `"winner": 1`
- **AND** `winner` is never `-1`

#### Scenario: Game-end includes per-player income and free_spaces

- **WHEN** `log_game_end` is called
- **THEN** the emitted JSON line contains `"p0"` and `"p1"` objects each with `"income"` and `"free_spaces"` matching the final state

### Requirement: Log output is NDJSON (one JSON object per line)

Each log line SHALL be a self-contained valid JSON object terminated by a newline. There SHALL be no trailing commas, no surrounding array brackets, and no blank lines between events.

#### Scenario: Each line is independently parseable

- **WHEN** a complete game log is produced
- **THEN** every line, read individually, parses as a valid JSON object

### Requirement: Batch runner plays a grid of games in a single process

The batch runner executable SHALL accept a set of setup ids, a per-setup game count `N`, and a single `--master-seed <n>`, and SHALL run every `(setup_id, game_index)` combination — `N` games per setup — to completion between two uniform-random agents within one process. It SHALL write one output stream to stdout, or to a file when `--output <file>` is given. The runner SHALL NOT spawn a separate process per game.

#### Scenario: Full grid is played

- **WHEN** the batch runner is invoked with 3 setups and `--games 100`
- **THEN** it exits with code 0 and the output contains exactly 300 per-game result records

#### Scenario: Every game reaches a terminal state

- **WHEN** the batch runner completes a grid
- **THEN** each per-game record reports a `winner` of `0` or `1` (never a draw or `-1`) and a positive `plies` count

### Requirement: Batch runner derives per-game seeds deterministically from the master seed

Each game's random agent SHALL be seeded by a deterministic mixing function of `(master_seed, setup_id, game_index)`, independent of the grid's ordering or size. The runner SHALL NOT reseed from a single stream advanced sequentially across games. The derived per-game seed SHALL be recorded in that game's output record so the game can be replayed independently with the single-game play driver.

#### Scenario: Whole batch is reproducible

- **WHEN** the batch runner is invoked twice with the same setups, game count, and `--master-seed`
- **THEN** the two output streams are byte-for-byte identical

#### Scenario: Per-game seed is order- and size-independent

- **WHEN** the game at `(setup_id, game_index)` is played in a batch of one setup, and again as part of a batch of several setups with the same `--master-seed`
- **THEN** the recorded seed and outcome for that `(setup_id, game_index)` are identical in both batches

#### Scenario: Recorded seed replays the game

- **WHEN** a game's recorded `setup_id` and `seed` are passed to the single-game play driver via `--setup` and `--seed`
- **THEN** the play driver's `game_end` scores and winner match that game's batch summary record

### Requirement: Batch runner emits a per-game summary record by default

By default the batch runner SHALL emit one NDJSON record per game of type `"game_summary"` containing `setup_id`, `seed`, `score_p0`, `score_p1`, `winner`, and `plies`, and SHALL NOT emit per-move lines. The `score_p0`, `score_p1`, and `winner` fields SHALL carry the same meaning and values as the single-game `game_end` event. A `--full` flag SHALL instead emit the complete per-game NDJSON stream (`game_start`, `move`, `game_end`) for every game in the grid.

#### Scenario: Summary record is well-formed and complete

- **WHEN** the batch runner runs in the default (summary) mode
- **THEN** every output line is a valid JSON object with `"event": "game_summary"` and includes `setup_id`, `seed`, `score_p0`, `score_p1`, `winner`, and `plies`

#### Scenario: Summary scores agree with full mode

- **WHEN** the same game is run in summary mode and in `--full` mode
- **THEN** the summary record's `score_p0`, `score_p1`, and `winner` equal the `game_end` event's values from the full run

#### Scenario: Full mode emits per-move logs

- **WHEN** the batch runner is invoked with `--full`
- **THEN** each game contributes exactly one `game_start` line, one or more `move` lines, and exactly one `game_end` line

### Requirement: Batch runner reports an error for invalid arguments

The batch runner SHALL print a usage message to stderr and exit with a non-zero code when the setup set, game count, or `--master-seed` is missing or non-numeric.

#### Scenario: Missing master seed

- **WHEN** the batch runner is invoked without `--master-seed`
- **THEN** it exits with a non-zero code and prints a usage message to stderr

