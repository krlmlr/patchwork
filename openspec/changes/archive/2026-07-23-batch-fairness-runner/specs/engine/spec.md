## ADDED Requirements

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
