## Why

The current NDJSON game logs omit important context needed for R analysis: the initial patch circle arrangement is not recorded in the game-start event, and per-move state summaries are too sparse (missing income, free spaces, total board value, and patch circle snapshot). Richer logs unlock the upcoming Tile Analysis phase, which requires full game context to model patch value and time cost.

## What Changes

- The `game_start` log event gains the initial patch circle as a 33-character string (patch names in circle order)
- The `move` log event gains four additional fields per active player: `income`, `free_spaces`, a `board_value` (buttons earned so far from income payments — i.e. the cumulative button income payout count), and the current patch circle snapshot (available patches in circle order)
- `game_end` gains final per-player `income` and `free_spaces` fields for completeness

## Capabilities

### New Capabilities

_(none)_

### Modified Capabilities

- `engine`: All three NDJSON events gain richer fields — `game_start` adds the initial patch circle string; `move` adds `income`, `free_spaces`, `board_value`, and a circle snapshot; `game_end` adds per-player `income` and `free_spaces`

## Impact

- `cpp/game_logger.hpp` / `cpp/game_logger.cpp`: All three logging functions gain new parameters or fields; `log_game_start` and `log_move` need access to the `GameSetup` to render circle strings
- `cpp/play_driver.cpp`: Call sites updated to pass `GameSetup` through to logging functions
- `cpp/tui/tui_main.cpp`: Call sites updated to pass `GameSetup` through to logging functions
- Existing log format changes are additive (new fields only) — no fields are removed or renamed
- Tests in `tests/` that assert NDJSON output strings will need updating to match new fields
