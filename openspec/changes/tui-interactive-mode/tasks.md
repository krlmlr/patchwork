## 1. Project Structure

- [ ] 1.1 Create `src/tui/` directory and add empty placeholder files: `history.hpp`, `history.cpp`, `display.hpp`, `display.cpp`, `input.hpp`, `input.cpp`, `launch.hpp`, `launch.cpp`, `tui_main.cpp`
- [ ] 1.2 Add new Meson executable target `patchwork-tui` in `src/meson.build` linking `src/tui/*.cpp` against the existing `patchwork_lib` static library
- [ ] 1.3 Verify `meson setup build && meson compile -C build patchwork-tui` succeeds with empty stubs

## 2. History Module

- [ ] 2.1 Define `History` class in `src/tui/history.hpp`: constructor accepts initial `GameState`; methods `push`, `undo`, `redo`, `current`, `can_undo`, `can_redo`
- [ ] 2.2 Implement `History` in `src/tui/history.cpp` using `std::vector<GameState>` and an `int` cursor
- [ ] 2.3 Write Catch2 tests in `tests/tui_history_test.cpp` covering: initial state, push, push-after-undo truncation, undo, redo, and boundary no-ops
- [ ] 2.4 Register the new test file in `tests/meson.build` and confirm `meson test -C build` passes

## 3. Display Module

- [ ] 3.1 Define `init_display()` in `src/tui/display.hpp`: checks terminal size (≥80×24 via `ioctl TIOCGWINSZ`), exits with error if too small
- [ ] 3.2 Implement `render_frame(const GameState&, const std::vector<std::string>& log)` that clears the terminal and prints the fixed four-pane ASCII frame (player stats, time track, patch circle up to 5 entries, quilt-board stub)
- [ ] 3.3 Implement `append_log(std::vector<std::string>& buf, std::string entry)` that appends to the log buffer and trims to 50 entries
- [ ] 3.4 Add unit tests in `tests/tui_display_test.cpp` for `append_log` buffer behaviour and for `render_frame` output containing expected substrings (redirect stdout to `std::ostringstream`); register in `tests/meson.build`

## 4. Input Module

- [ ] 4.1 Define `Command` variant in `src/tui/input.hpp`: `BuyPatch{int index}`, `Advance{}`, `Undo{}`, `Redo{}`, `Quit{}`
- [ ] 4.2 Implement `RawMode` RAII class in `src/tui/input.cpp`: saves `termios` on construction, restores on destruction, registers `atexit` handler
- [ ] 4.3 Implement `read_command()` that reads one character in raw mode and maps it to a `Command`, ignoring unrecognised keys
- [ ] 4.4 Implement `is_legal(const Command&, const GameState&)` helper that checks whether the resolved `Move` is in `generate_moves(state)`

## 5. Launch Module

- [ ] 5.1 Implement `run_launch_screen()` in `src/tui/launch.cpp`: prompts for setup index (0–99, default 0) and seed (default 42) using normal cooked-mode line input, validates each field, re-prompts on invalid input
- [ ] 5.2 Return a `LaunchConfig{int setup_index, uint64_t seed}` struct from `run_launch_screen()`

## 6. Main Game Loop

- [ ] 6.1 Implement `tui_main.cpp`: call `init_display`, `run_launch_screen`, construct initial `GameState` from `kGameSetups[setup_index]` with `GameSetup`, seed `RandomAgent`, construct `History`
- [ ] 6.2 Enter the main loop: call `render_frame`, call `read_command`, dispatch `Command` — apply legal player moves, trigger opponent move after player moves, push new state to `History`, append log entries
- [ ] 6.3 Handle `Undo` / `Redo` commands by calling `history.undo()` / `history.redo()` and re-rendering without advancing the opponent
- [ ] 6.4 Detect terminal game state after each move and break the loop
- [ ] 6.5 On `Quit` or terminal state: clear terminal and print result summary (player scores, bonus flag, winner declaration)

## 7. Integration and Polish

- [ ] 7.1 Run the full test suite (`meson test -C build`) and confirm all existing tests and new TUI tests pass
- [ ] 7.2 Manually play a short game end-to-end: verify undo/redo, patch circle display, log scrolling, and result summary are correct
- [ ] 7.3 Test on a terminal sized exactly 80×24 and confirm layout is not clipped
