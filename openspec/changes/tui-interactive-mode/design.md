## Context

The engine has complete game logic (move generation, application, terminal detection, logging) but no human-facing interface. All interaction currently happens through the programmatic `PlayDriver` loop and file-based NDJSON logs. The TUI phase introduces a text-mode UI so a human can play against a random (or future heuristic) agent, explore game states, and validate engine behaviour interactively. The design must stay dependency-light and fit the existing C++23 / Meson project structure.

## Goals / Non-Goals

**Goals:**
- Render current game state in ASCII art on a standard terminal: buttons, income, free spaces, time-track positions, and the visible patch circle
- Scrolling event log alongside the board view
- Single-keypress input loop: digit keys to buy a patch by circle index, `a` / Space to advance, `z`/`Z` for undo/redo, `q` to quit
- Unlimited undo/redo via a move history stack
- Launch screen: pick game setup index, random seed, and opponent agent before starting
- TUI preview (ASCII mockup) included in this design document
- New Meson executable target `patchwork-tui` linking `src/tui/` against the engine static library
- Catch2 unit tests for history stack and display helpers

**Non-Goals:**
- Full quilt-board ASCII art (reserved for a future phase; area shown as placeholder)
- Networked or multiplayer sessions
- Mouse support
- Windows console (UTF-8 ANSI terminal assumed)
- Saving/loading sessions to disk

## TUI Preview

```
╔══════════════════════════════════════════════════════════════════╗
║  PATCHWORK  —  seed 42, setup 0                                  ║
╠══════════════════════╦═══════════════════════════════════════════╣
║  Player 1   Player 2 ║  Patch circle (3 available)               ║
║  btn  14    btn  11  ║  [0] ◆ cost 2  time 1  income 0           ║
║  inc   3    inc   2  ║  [1] ▲ cost 4  time 2  income 1           ║
║  free 74   free 81   ║  [2] ● cost 3  time 3  income 2           ║
╠══════════════════════╣  ...                                       ║
║  Time track          ╠═══════════════════════════════════════════╣
║  0  .  .  .  .  .    ║  Event log                                 ║
║  P1──────┘           ║  > P1 bought patch [1]  (+1 income)       ║
║  P2─────────────┘    ║  > P2 advanced (+4 buttons)               ║
║  . . 54 . . . 58     ║  > P1 earned leather patch                ║
╠══════════════════════╣  > P2 bought patch [0]                    ║
║  Quilt board (stub)  ║                                            ║
║  [reserved]          ╚═══════════════════════════════════════════╣
║                      ║  [0-2] buy patch  [a] advance  [z] undo  ║
╚══════════════════════╩══════════════════════════════════════════╝
```

## Decisions

### 1. ANSI escape codes directly, no ncurses

**Decision:** Use raw ANSI escape codes (`\033[H\033[2J` clear, `\033[<r>;<c>H` cursor positioning) and `termios` for raw terminal mode. No third-party TUI library.

**Rationale:** ncurses is a non-trivial Meson/pkg-config dependency and is overkill for a fixed-layout display. The layout is static (no scrolling regions needed beyond a simple log buffer). ANSI codes are universally supported in any POSIX terminal and avoid a new entry in the advisory database. The display is always fully redrawn on each frame (clear + reprint), keeping render logic simple.

**Alternative considered:** `ftxui` (C++ TUI library) — powerful but adds a CMake-based dependency that conflicts with Meson-first philosophy.

### 2. History as a `std::vector<GameState>` with a cursor

**Decision:** `History` holds a `std::vector<GameState>` and an `int current` index. Undo decrements the index; redo increments it; a new move truncates everything above the cursor and appends.

**Rationale:** `GameState` is compact (two 128-bit player states + one 64-bit shared word = 40 bytes). Storing the full state on each move is trivially cheap compared to storing move lists and replaying from the start. This also makes undo/redo O(1) and avoids replaying move sequences.

**Alternative considered:** Store only moves and replay — O(n) undo, complicates the history interface, no benefit for this state size.

### 3. Single-file `tui_main.cpp` entry point, modular headers

**Decision:** Four translation units under `src/tui/`: `display.cpp`, `input.cpp`, `history.cpp`, `launch.cpp`, plus a `tui_main.cpp` entry point. Each has a corresponding `.hpp`.

**Rationale:** Keeps each concern independently testable. `history.cpp` and `display.cpp` have no terminal I/O and are straightforward to unit-test with Catch2.

### 4. Terminal raw mode scoped with RAII

**Decision:** A `RawMode` RAII guard in `input.hpp` saves/restores `termios` settings on construction/destruction.

**Rationale:** Ensures the terminal is always restored to cooked mode on exit, exception, or `SIGTERM` (via `atexit`).

### 5. Opponent is always the random agent (for this phase)

**Decision:** The launch screen lets the user pick a game setup index and seed; the opponent is always `RandomAgent`. Future phases add agent selection.

**Rationale:** No agent choice is needed until Random Sampling Agents phase. Keeping scope narrow avoids prematurely abstracting the agent interface.

## Risks / Trade-offs

- **Terminal size assumptions** → The fixed layout assumes at least 80×24. Render code will check terminal size at startup and abort with a clear error if the terminal is too small.
- **SIGWINCH / resize** → Window resize during play is not handled; the display will be garbled until the next key press triggers a full redraw. Acceptable for this phase.
- **Raw mode on CI / non-TTY** → `patchwork-tui` checks `isatty(STDIN_FILENO)` and exits gracefully if not running in a TTY. The test suite for `history` and `display` does not use raw mode.

## Migration Plan

1. Add `src/tui/` directory with four modules and entry point
2. Add new Meson executable in `src/meson.build` depending on `patchwork_lib`
3. Add Catch2 test file `tests/tui_history_test.cpp`
4. No changes to existing targets; existing tests continue to pass

## Open Questions

- Should the patch circle display wrap after 3 patches or show all available? → show up to 5 visible patches (the window the rules define), truncate with "…" if fewer are available at end of game.
- Should `z`/`Z` be undo/redo, or arrow keys? → `z` (undo) / `Z` (redo) preferred for single-hand use; also support `u` / `r` as aliases.
