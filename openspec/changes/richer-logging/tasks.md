## 1. Update Logger API

- [ ] 1.1 Add `const GameSetup&` parameter to `log_game_start` in `cpp/game_logger.hpp` and `cpp/game_logger.cpp`
- [ ] 1.2 Add `const GameSetup&` parameter to `log_move` in `cpp/game_logger.hpp` and `cpp/game_logger.cpp`

## 2. Implement game_start Enhancements

- [ ] 2.1 In `log_game_start`, emit a `"circle"` field: iterate `GameSetup::circle()` and write each patch name character to produce a 33-char string

## 3. Implement move Enhancements

- [ ] 3.1 In `log_move`, emit `"income"` and `"free_spaces"` for the active player from `new_state.player(player)`
- [ ] 3.2 In `log_move`, emit `"board_value"` as `new_state.player(player).buttons() - 5`
- [ ] 3.3 In `log_move`, emit `"circle"` by iterating from `new_state.circle_marker()` through all 33 positions (mod 33), appending the patch name for each position where `new_state.patch_available(id)` is true using the `GameSetup` circle

## 4. Implement game_end Enhancements

- [ ] 4.1 In `log_game_end`, restructure output to emit `"p0"` and `"p1"` objects each containing `"income"` and `"free_spaces"` alongside the existing `"score_p0"`, `"score_p1"`, and `"winner"` fields

## 5. Update Call Sites

- [ ] 5.1 Update `cpp/play_driver.cpp`: pass `setup` to `log_game_start`
- [ ] 5.2 Update `cpp/play_driver.cpp`: pass `setup` to `log_move`
- [ ] 5.3 Update `cpp/tui/tui_main.cpp`: pass setup to `log_game_start` and both `log_move` call sites

## 6. Update Tests

- [ ] 6.1 Find all Catch2 tests in `tests/` that assert NDJSON output strings for `game_start`, `move`, or `game_end` events
- [ ] 6.2 Update each such test to include the new fields (`circle` in game_start, `income`/`free_spaces`/`board_value`/`circle` in move, `p0`/`p1` objects with `income`/`free_spaces` in game_end)
- [ ] 6.3 Add new Catch2 test cases covering: circle string length is 33 at game start; circle shrinks after a buy-patch move; `board_value` is 0 for a fresh player; `game_end` contains `income` and `free_spaces` for both players
- [ ] 6.4 Build and run the full test suite (`meson test` or equivalent) and confirm all tests pass
