## Why

The current NDJSON game logs omit important context needed for R analysis: the initial patch circle arrangement is not recorded in the game-start event, and per-move state summaries are too sparse (missing income, free spaces, board value, projected income/score, and patch circle snapshot). Richer logs unlock the upcoming Tile Analysis phase, which requires full game context to model patch value and time cost.

## What Changes

- The `game_start` log event gains the initial patch circle as a 33-character string (patch names in circle order, always ending with `'2'`)
- The `move` log event gains additional per-player fields: `income`, `free_spaces`, `board_value` (`buttons - 2 × free_spaces`, the game-score formula), `projected_income` (`income × remaining income-trigger count`), `projected_score` (`buttons + projected_income - 2 × free_spaces`), the current patch circle snapshot (available patches in circle order from the marker), and `patch_symbol` (single-character patch name for buy-patch moves)
- `game_end` gains final per-player `income` and `free_spaces` fields for completeness
- `make_setup()` is simplified to delegate to the pre-generated `kGameSetups` table (no duplicate setup-generation logic)

## Capabilities

### New Capabilities

_(none)_

### Modified Capabilities

- `engine`: All three NDJSON events gain richer fields — `game_start` adds the initial patch circle string; `move` adds `income`, `free_spaces`, `board_value`, `projected_income`, `projected_score`, `patch_symbol`, and a circle snapshot; `game_end` adds per-player `income` and `free_spaces`

## Impact

- `cpp/game_logger.hpp` / `cpp/game_logger.cpp`: All three logging functions gain new parameters or fields; `log_game_start` and `log_move` need access to the `GameSetup` to render circle strings
- `cpp/game_setups.hpp`: `make_setup()` now delegates to `kGameSetups` — duplicate rotation logic removed
- `cpp/play_driver.cpp`: Call sites updated to pass `GameSetup` through to logging functions
- `cpp/tui/tui_main.cpp`: Call sites updated to pass `GameSetup` through to logging functions
- Existing log format changes are additive (new fields only) — no fields are removed or renamed
- Tests in `tests/` that assert NDJSON output strings will need updating to match new fields
