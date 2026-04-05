## Context

The engine has complete game logic (move generation, application, terminal detection, logging) but no human-facing interface. All interaction currently happens through the programmatic `PlayDriver` loop and file-based NDJSON logs. The TUI phase introduces a text-mode UI so a human can play against a random (or future heuristic) agent, explore game states, and validate engine behaviour interactively. The design must stay dependency-light and fit the existing C++23 / Meson project structure.

## Goals / Non-Goals

**Goals:**
- Render current game state in ASCII art on a standard terminal: buttons, income, free spaces, time-track positions, the full patch circle (all 33 patch characters as a sequence with a circle-marker indicator), and adaptive detail lines for the next 3+ buyable patches
- Two reserved 9×9 quilt board areas (one per player) displaying `?` in simplified mode — sized so the full quilt game requires no layout changes
- Scrolling event log alongside the board view with horizontal scrolling and an optional line-wrap toggle
- Single-keypress input loop: digit keys to buy a patch by circle index, `a` / Space to advance, `z`/`Z` for undo/redo, `q` to quit
- Unlimited undo/redo via a move history stack that preserves the full RNG state alongside each game state for deterministic replay
- Minimum terminal width 80 columns; layout expands to use additional width when available
- Launch screen: pick game setup index and random seed before starting
- TUI preview (ASCII mockup) included in this design document
- New Meson executable target `patchwork-tui` linking `src/tui/` against the engine static library
- Catch2 unit tests for history stack and display helpers

**Non-Goals:**
- Full quilt-board ASCII art (cell contents shown as `?` until a placement phase is added)
- Networked or multiplayer sessions
- Mouse support
- Windows console (UTF-8 ANSI terminal assumed)
- Saving/loading sessions to disk

## TUI Preview

Layout at 80 columns (minimum); extra columns widen the log pane and the time-track bar.

```
PATCHWORK — seed 42 / setup 0                                         [P1]
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Circle: ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567  (^ = buy window start)
                ^
  [0] A  cost 3  time 2  income 1     [0-x] buy  [a] advance  [q] quit
  [1] B  cost 5  time 3  income 2     [z/u] undo  [Z/r] redo  [</> ] log scroll
  [2] C  cost 2  time 1  income 0     [↵] toggle log wrap
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  P1: btn 14  inc  3  pos 12  fr 74   P2: btn 11  inc  2  pos 20  fr 81
  Time:  ──P1────────────────P2──────────────────────────────  0..53
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  P1 quilt (9×9)    P2 quilt (9×9)    Event log
  ?????????         ?????????         > P1 bought [1] (+1 income)
  ?????????         ?????????         > P2 advanced (+4 buttons)
  ?????????         ?????????         > P1 earned leather patch
  ?????????         ?????????         > P2 bought [0]
  ?????????         ?????????         >
  ?????????         ?????????         [< >] scroll  [↵] toggle wrap
  ?????????         ?????????
  ?????????         ?????????
  ?????????         ?????????
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

Column budget at 80: quilt section ~30 cols (`  ?????????   ?????????   `), log ~48 cols. At 120 cols the log pane gains ~40 additional columns automatically.

## Decisions

### 1. ANSI escape codes directly, no ncurses

**Decision:** Use raw ANSI escape codes (`\033[H\033[2J` clear, `\033[<r>;<c>H` cursor positioning) and `termios` for raw terminal mode. No third-party TUI library.

**Rationale:** ncurses is a non-trivial Meson/pkg-config dependency and is overkill for a fixed-layout display. The layout is static (no scrolling regions needed beyond a simple log buffer). ANSI codes are universally supported in any POSIX terminal and avoid a new entry in the advisory database. The display is always fully redrawn on each frame (clear + reprint), keeping render logic simple.

**Alternative considered:** `ftxui` (C++ TUI library) — powerful but adds a CMake-based dependency that conflicts with Meson-first philosophy.

### 2. History stores `(GameState, RngState)` pairs with a cursor

**Decision:** `History` holds a `std::vector<HistoryEntry>` where each entry is a `(GameState, RngState)` pair, plus an `int current` index. `RngState` captures a snapshot of the random agent's `std::mt19937_64` state at the moment the entry was recorded. Undo decrements the cursor; redo increments it and restores the saved `RngState` to the agent so the opponent's next move is identical to what was played originally. A new move truncates everything above the cursor and appends.

**Rationale:** `GameState` is compact (~40 bytes). `std::mt19937_64::state_size` is 312 `uint64_t` words (~2.5 KB). Even at 200 history entries this is ~500 KB — negligible. Storing the full RNG state makes redo deterministic without replaying move sequences, and avoids any dependency on a move-log approach. This directly fulfils the "preserve engine random seed" requirement.

**Alternative considered:** Store only moves and replay from the start — O(n) redo, and still requires a seeded-from-start RNG snapshot at the initial entry; no advantage over full-state storage.

### 6. Full patch circle display with adaptive detail

**Decision:** The circle section always shows the full 33-character patch sequence as a single line (using single-char patch names from the patch catalog), with a `^` marker on the line below indicating the current buy-window start. Immediately below, a variable number of detail lines (at least 3, more if vertical space allows) list the buyable patches with cost, time cost, and income. On terminals wider than 80 columns the detail section gains one more row per N extra columns (where N is tuned empirically).

**Rationale:** Showing all 33 characters gives the player a full strategic view of the circle at a glance. The adaptive detail respects the "space allowing" constraint from the roadmap description while always guaranteeing at least 3 buy options are described.

### 7. Two 9×9 quilt areas; `?` placeholder for simplified mode

**Decision:** The lower-left pane is permanently split into two 9×9 grids (one per player). Each cell is a single character wide. In simplified mode every cell displays `?`. When a full quilt board is added in a later phase, cells are replaced with patch characters — no layout changes required.

**Rationale:** Reserving the exact final quilt dimensions today means the frame geometry is fixed for the lifetime of the project. `?` clearly communicates "not yet tracked" to the user without pretending the data is absent.

### 8. Responsive layout: 80-column baseline, expand if wider

**Decision:** The rendering function queries the current terminal width on each frame via `ioctl TIOCGWINSZ`. The log pane gets all columns beyond the fixed left section (quilt + stats + separators ≈ 32 cols). The time-track bar scales to fill the available width. A minimum of 80 columns is enforced at startup.

**Rationale:** Makes the TUI immediately useful on wider terminals (100, 120, 160 cols) without a separate wide-mode code path.

### 9. Log horizontal scrolling and wrap toggle

**Decision:** The log pane maintains a horizontal scroll offset (in characters). `<` and `>` keys shift it left/right. A `↵` (Enter) key toggles word-wrap mode; in wrap mode the scroll offset is ignored. The scroll offset resets to 0 on each new log entry so the most recent line is always fully visible by default.

**Rationale:** Long move descriptions or patch names can exceed the log pane width. Horizontal scrolling is simpler than truncation and lets users read the full text. Wrap mode is an alternative for users who prefer it.

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

- Should `z`/`Z` be undo/redo, or arrow keys? → `z` (undo) / `Z` (redo) preferred for single-hand use; also support `u` / `r` as aliases.
- Adaptive circle detail rows: how many extra rows per extra column? → Empirically tune during implementation; start with `floor((terminal_width - 80) / 10)` bonus rows, minimum 0.
