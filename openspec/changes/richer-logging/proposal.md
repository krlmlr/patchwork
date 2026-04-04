## Why

The current NDJSON game logs omit important context needed for R analysis: the initial patch circle arrangement is not recorded in the game-start event, and per-move state summaries are too sparse (missing income, free spaces, total board value, and patch circle snapshot). Richer logs unlock the upcoming Tile Analysis phase, which requires full game context to model patch value and time cost.

## What Changes

- The `game_start` log event gains the initial patch circle as a 33-character string (patch names in circle order)
- The `move` log event gains four additional fields per active player: `income`, `free_spaces`, a `board_value` (buttons earned so far from income payments — i.e. the cumulative button income payout count), and the current patch circle snapshot (available patches in circle order)
- `game_end` gains final per-player `income` and `free_spaces` fields for completeness

## Capabilities

### New Capabilities

- `game-start-logging`: Extended `game_start` NDJSON event that includes the full initial patch circle (33-char patch-name string derived from `GameSetup`)
- `move-logging`: Extended `move` NDJSON event that includes post-move per-player summary (income, free_spaces, board_value) and the current patch circle state (available patches in circle order using `SimplifiedGameState::patch_available` and the `GameSetup` circle)
- `game-end-logging`: Extended `game_end` NDJSON event that includes final per-player `income` and `free_spaces`

### Modified Capabilities

_(none — no existing spec-level requirements are changing; this is additive)_

## Impact

- `src/game_logger.hpp` / `src/game_logger.cpp`: All three logging functions gain new parameters or fields; `log_game_start` and `log_move` need access to the `GameSetup` to render circle strings
- `src/play_driver.cpp`: Call sites updated to pass `GameSetup` through to logging functions
- Existing log format changes are additive (new fields only) — no fields are removed or renamed
- Tests in `tests/` that assert NDJSON output strings will need updating to match new fields
