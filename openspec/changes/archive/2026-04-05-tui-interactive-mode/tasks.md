## 1. Project Structure

- [x] 1.1 Create `cpp/tui/` directory and add empty placeholder files: `history.hpp`, `history.cpp`, `display.hpp`, `display.cpp`, `input.hpp`, `input.cpp`, `launch.hpp`, `launch.cpp`, `tui_main.cpp`
- [x] 1.2 Add new Meson executable target `patchwork-tui` in `cpp/meson.build` linking `cpp/tui/*.cpp` against the existing `patchwork_lib` static library
- [x] 1.3 Verify `meson setup build && meson compile -C build patchwork-tui` succeeds with empty stubs

## 2. History Module

- [x] 2.1 Define `HistoryEntry` struct in `cpp/tui/history.hpp`: holds `GameState` + `RngState` (a snapshot of `std::mt19937_64` state)
- [x] 2.2 Define `History` class: constructor accepts initial `GameState` and `RngState`; methods `push`, `undo`, `redo`, `current_state`, `current_rng`, `can_undo`, `can_redo`
- [x] 2.3 Implement `History` in `cpp/tui/history.cpp` using `std::vector<HistoryEntry>` and an `int` cursor
- [x] 2.4 Write Catch2 tests in `tests/tui_history_test.cpp` covering: initial entry, push, push-after-undo truncation, undo, redo, boundary no-ops, and deterministic redo (verify `current_rng()` after redo returns the saved RNG state)
- [x] 2.5 Register the new test file in `tests/meson.build` and confirm `meson test -C build` passes

## 3. Display Module

- [x] 3.1 Define `init_display()` in `cpp/tui/display.hpp`: queries terminal size via `ioctl TIOCGWINSZ`; exits with error message if fewer than 80×24; detects `TERM=dumb` / `NO_COLOR` / `--no-color` and sets a `color_enabled` flag
- [x] 3.2 Implement box-drawn frame: use `┌─┐│├─┤└─┘┬┴┼` for all borders and section dividers; narrow (80–159 col) and wide (≥160 col) layouts
- [x] 3.3 Implement full patch circle line: render all 33 patch characters in circle order (bought patches shown as `.`), with `^` marker line below at the buy-window start position
- [x] 3.4 Implement adaptive detail lines: show at least 3 buyable-patch detail rows (index, name, cost, time, income); add `floor((width-80)/10)` extra rows on wider terminals; color affordable rows green, unaffordable rows dim
- [x] 3.5 Implement two 9×9 quilt board areas: each cell displays `?` in simplified mode; areas are labelled "P1 quilt" and "P2 quilt" and their dimensions are fixed for future full-quilt use
- [x] 3.6 Implement responsive layout: narrow — event-log pane width = `terminal_width - 32`; wide (≥160) — left column ≈65 cols, right column = remaining; time-track bar scales to fill available width
- [x] 3.7 Apply ANSI 16-color using named constants in `display.hpp`: P1 bright cyan, P2 bright yellow, active-player bold, event-log `>` green, error bold red; NDJSON pane: structural chars dim, player values use player colors, `buy_patch` green, `advance` cyan, `game_start`/`game_end` bold, `winner` values use player colors; skip color codes when `color_enabled` is false
- [x] 3.8 Implement resizable NDJSON log pane: default 5 lines; `m` toggles minimize/restore; `f` maximizes; `h` semi-maximizes (`floor(max/2)`); `,`/`.` dec/inc by 1 (clamped); header bar always visible showing height and shortcuts
- [x] 3.9 Implement `render_frame(const GameState&, const LogState&, const NdjsonState&)` that clears the terminal and prints the full frame
- [x] 3.10 Implement `append_log(LogState&, std::string)` that appends to the log buffer (max 50 entries) and resets the horizontal scroll offset to 0
- [x] 3.11 Implement log horizontal scrolling (`<` / `>` keys update offset) and wrap toggle (`w` key toggles wrap mode; in wrap mode offset is ignored)
- [x] 3.12 Add unit tests in `tests/tui_display_test.cpp` for: `append_log` buffer trimming, scroll-offset reset on new entry, circle line length, marker placement, quilt grid dimensions, log wrap vs. scroll rendering, NDJSON pane height state machine, color suppression; register in `tests/meson.build`
- [x] 3.13 Add snapshot tests for a few moves into a simple game (output without color) for 80, 120 and 160 columns, verifying the full frame layout and content
- [x] 3.14 Add snapshot tests for color output

## 4. Input Module

- [x] 4.1 Define `Command` variant in `cpp/tui/input.hpp`: `BuyPatch{int index}`, `Advance{}`, `Undo{}`, `Redo{}`, `ScrollLogLeft{}`, `ScrollLogRight{}`, `ToggleLogWrap{}`, `NdjsonToggleMinimize{}`, `NdjsonMaximize{}`, `NdjsonSemiMaximize{}`, `NdjsonDecrLines{}`, `NdjsonIncrLines{}`, `Quit{}`
- [x] 4.2 Implement `RawMode` RAII class in `cpp/tui/input.cpp`: saves `termios` on construction, restores on destruction, registers `atexit` handler
- [x] 4.3 Implement `read_command()` that reads one character in raw mode and maps it to a `Command`: digits `0`–`9` → `BuyPatch`, `a`/Space → `Advance`, `z`/`u` → `Undo`, `Z`/`r` → `Redo`, `<` → `ScrollLogLeft`, `>` → `ScrollLogRight`, `w` → `ToggleLogWrap`, `m` → `NdjsonToggleMinimize`, `f` → `NdjsonMaximize`, `h` → `NdjsonSemiMaximize`, `,` → `NdjsonDecrLines`, `.` → `NdjsonIncrLines`, `q`/`Q` → `Quit`; ignore unrecognised keys
- [x] 4.4 Implement `is_legal(const Command&, const GameState&)` helper that checks whether the resolved `Move` is in `generate_moves(state)`

## 5. Launch Module

- [x] 5.1 Implement `run_launch_screen()` in `cpp/tui/launch.cpp`: prompts for setup index (0–99, default 0) and seed (default 42) using normal cooked-mode line input, validates each field, re-prompts on invalid input
- [x] 5.2 Return a `LaunchConfig{int setup_index, uint64_t seed}` struct from `run_launch_screen()`

## 6. Main Game Loop

- [x] 6.1 Implement `tui_main.cpp`: parse `--no-color` flag; call `init_display`, `run_launch_screen`, construct initial `GameState` from `kGameSetups[setup_index]` with `GameSetup`, seed `RandomAgent`, snapshot initial `RngState`, construct `History` and initial `NdjsonState`
- [x] 6.2 Enter the main loop: call `render_frame`, call `read_command`, dispatch `Command` — snapshot RNG state before opponent move, apply legal player moves, trigger opponent move, push `(new_state, pre_opponent_rng_state)` to `History`, append log entries
- [x] 6.3 Handle `Undo` / `Redo` commands: call `history.undo()` / `history.redo()`, restore agent to `history.current_rng()`, re-render
- [x] 6.4 Handle `ScrollLogLeft`, `ScrollLogRight`, `ToggleLogWrap`: update `LogState` and re-render without advancing game state
- [x] 6.5 Handle `NdjsonToggleMinimize`, `NdjsonMaximize`, `NdjsonSemiMaximize`, `NdjsonDecrLines`, `NdjsonIncrLines`: update `NdjsonState` and re-render without advancing game state
- [x] 6.6 Detect terminal game state after each move and break the loop
- [x] 6.7 On `Quit` or terminal state: clear terminal and print result summary (player scores, bonus flag, winner declaration)

## 7. Integration and Polish

- [x] 7.1 Run the full test suite (`meson test -C build`) and confirm all existing tests and new TUI tests pass
- [ ] 7.2 Manually play a short game end-to-end: verify undo/redo (including deterministic opponent redo), full circle display, adaptive detail rows, 9×9 quilt `?` grids, log scrolling, wrap toggle, NDJSON panel resizing (min/max/semi/fine-tune), and result summary
- [ ] 7.3 Test on a terminal sized exactly 80×24 (narrow layout)
- [ ] 7.4 Test on a 120-column terminal (narrow layout, wider log pane)
- [ ] 7.5 Test on a 160-column terminal (wide layout: circle left, quilts right, expanded NDJSON)
- [ ] 7.6 Test with `NO_COLOR` set and verify no ANSI escape codes in output
