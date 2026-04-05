## 1. Project Structure

- [ ] 1.1 Create `src/tui/` directory and add empty placeholder files: `history.hpp`, `history.cpp`, `display.hpp`, `display.cpp`, `input.hpp`, `input.cpp`, `launch.hpp`, `launch.cpp`, `tui_main.cpp`
- [ ] 1.2 Add new Meson executable target `patchwork-tui` in `src/meson.build` linking `src/tui/*.cpp` against the existing `patchwork_lib` static library
- [ ] 1.3 Verify `meson setup build && meson compile -C build patchwork-tui` succeeds with empty stubs

## 2. History Module

- [ ] 2.1 Define `HistoryEntry` struct in `src/tui/history.hpp`: holds `GameState` + `RngState` (a snapshot of `std::mt19937_64` state)
- [ ] 2.2 Define `History` class: constructor accepts initial `GameState` and `RngState`; methods `push`, `undo`, `redo`, `current_state`, `current_rng`, `can_undo`, `can_redo`
- [ ] 2.3 Implement `History` in `src/tui/history.cpp` using `std::vector<HistoryEntry>` and an `int` cursor
- [ ] 2.4 Write Catch2 tests in `tests/tui_history_test.cpp` covering: initial entry, push, push-after-undo truncation, undo, redo, boundary no-ops, and deterministic redo (verify `current_rng()` after redo returns the saved RNG state)
- [ ] 2.5 Register the new test file in `tests/meson.build` and confirm `meson test -C build` passes

## 3. Display Module

- [ ] 3.1 Define `init_display()` in `src/tui/display.hpp`: queries terminal size via `ioctl TIOCGWINSZ`; exits with error message if fewer than 80×24
- [ ] 3.2 Implement full patch circle line: render all 33 patch characters in circle order (bought patches shown as `.`), with `^` marker line below at the buy-window start position
- [ ] 3.3 Implement adaptive detail lines: show at least 3 buyable-patch detail rows (index, name, cost, time, income); add `floor((width-80)/10)` extra rows on wider terminals
- [ ] 3.4 Implement two 9×9 quilt board areas: each cell displays `?` in simplified mode; areas are labelled "P1 quilt" and "P2 quilt" and their dimensions are fixed for future full-quilt use
- [ ] 3.5 Implement responsive layout: log pane width = `terminal_width - 32` (minimum 1); time-track bar scales to fill available width
- [ ] 3.6 Implement `render_frame(const GameState&, const LogState&)` that clears the terminal and prints the full five-section frame
- [ ] 3.7 Implement `append_log(LogState&, std::string)` that appends to the log buffer (max 50 entries) and resets the horizontal scroll offset to 0
- [ ] 3.8 Implement log horizontal scrolling (`<` / `>` keys update offset) and wrap toggle (Enter key toggles wrap mode; in wrap mode offset is ignored)
- [ ] 3.9 Add unit tests in `tests/tui_display_test.cpp` for: `append_log` buffer trimming, scroll-offset reset on new entry, circle line length, marker placement, quilt grid dimensions, log wrap vs. scroll rendering; register in `tests/meson.build`

## 4. Input Module

- [ ] 4.1 Define `Command` variant in `src/tui/input.hpp`: `BuyPatch{int index}`, `Advance{}`, `Undo{}`, `Redo{}`, `ScrollLogLeft{}`, `ScrollLogRight{}`, `ToggleLogWrap{}`, `Quit{}`
- [ ] 4.2 Implement `RawMode` RAII class in `src/tui/input.cpp`: saves `termios` on construction, restores on destruction, registers `atexit` handler
- [ ] 4.3 Implement `read_command()` that reads one character in raw mode and maps it to a `Command`: digits `0`–`9` → `BuyPatch`, `a`/Space → `Advance`, `z`/`u` → `Undo`, `Z`/`r` → `Redo`, `<` → `ScrollLogLeft`, `>` → `ScrollLogRight`, Enter → `ToggleLogWrap`, `q`/`Q` → `Quit`; ignore unrecognised keys
- [ ] 4.4 Implement `is_legal(const Command&, const GameState&)` helper that checks whether the resolved `Move` is in `generate_moves(state)`

## 5. Launch Module

- [ ] 5.1 Implement `run_launch_screen()` in `src/tui/launch.cpp`: prompts for setup index (0–99, default 0) and seed (default 42) using normal cooked-mode line input, validates each field, re-prompts on invalid input
- [ ] 5.2 Return a `LaunchConfig{int setup_index, uint64_t seed}` struct from `run_launch_screen()`

## 6. Main Game Loop

- [ ] 6.1 Implement `tui_main.cpp`: call `init_display`, `run_launch_screen`, construct initial `GameState` from `kGameSetups[setup_index]` with `GameSetup`, seed `RandomAgent`, snapshot initial `RngState`, construct `History`
- [ ] 6.2 Enter the main loop: call `render_frame`, call `read_command`, dispatch `Command` — snapshot RNG state before opponent move, apply legal player moves, trigger opponent move, push `(new_state, pre_opponent_rng_state)` to `History`, append log entries
- [ ] 6.3 Handle `Undo` / `Redo` commands: call `history.undo()` / `history.redo()`, restore agent to `history.current_rng()`, re-render
- [ ] 6.4 Handle `ScrollLogLeft`, `ScrollLogRight`, `ToggleLogWrap`: update `LogState` and re-render without advancing game state
- [ ] 6.5 Detect terminal game state after each move and break the loop
- [ ] 6.6 On `Quit` or terminal state: clear terminal and print result summary (player scores, bonus flag, winner declaration)

## 7. Integration and Polish

- [ ] 7.1 Run the full test suite (`meson test -C build`) and confirm all existing tests and new TUI tests pass
- [ ] 7.2 Manually play a short game end-to-end: verify undo/redo (including deterministic opponent redo), full circle display, adaptive detail rows, 9×9 quilt `?` grids, log scrolling, wrap toggle, and result summary
- [ ] 7.3 Test on a terminal sized exactly 80×24 and confirm layout is not clipped
- [ ] 7.4 Test on a 120-column terminal and confirm the log pane and time-track bar expand correctly
