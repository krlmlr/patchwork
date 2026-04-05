## Context

The engine has complete game logic (move generation, application, terminal detection, logging) but no human-facing interface. All interaction currently happens through the programmatic `PlayDriver` loop and file-based NDJSON logs. The TUI phase introduces a text-mode UI so a human can play against a random (or future heuristic) agent, explore game states, and validate engine behaviour interactively. The design must stay dependency-light and fit the existing C++23 / Meson project structure.

## Goals / Non-Goals

**Goals:**

- Render current game state in ASCII art on a standard terminal: buttons, income, free spaces, time-track positions, the full patch circle (all 33 patch characters as a sequence with a circle-marker indicator), and adaptive detail lines for the next 3+ buyable patches
- Two reserved 9×9 quilt board areas (one per player) displaying `?` in simplified mode — sized so the full quilt game requires no layout changes
- Scrolling event log alongside the board view with horizontal scrolling and an optional line-wrap toggle
- Dedicated NDJSON log pane below the main frame, resizable with keyboard shortcuts: minimize, semi-maximize, maximize, and single-line increment/decrement
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

### Narrow layout — 80 columns (minimum)

Extra columns beyond 80 widen the event-log pane and the time-track bar.

```txt
┌─ PATCHWORK ── seed 42 / setup 0 ────────────────────────────────── ▶ P1 ─┐
│ Circle: ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567                                │
│                  ^                                                       │
│  [0] A  cost 3  time 2  inc 1   [0-x]buy  [a]adv      [q]quit            │
│  [1] B  cost 5  time 3  inc 2   [z/u]undo [Z/r]redo   [</>]log  [w]wrap  │
│  [2] C  cost 2  time 1  inc 0   [m]ndjson↕  [f]ndjson⤢  [h]ndjson½↕      │
│                                 [,]decrLines  [.]incrLines               │
├───────────────────────────────────┬──────────────────────────────────────┤
│ P1  btn 14  inc 3  pos 12  fr 74  │ P2  btn 11  inc 2  pos 20  fr 81     │
├─────────────────┬─────────────────┴──────────────────────────────────────┤
│ P1 quilt (9×9)  │ P2 quilt (9×9)   Event log                             │
│ ?????????       │ ?????????        > P1 bought [1] (+1 income)           │
│ ?????????       │ ?????????        > P2 advanced (+4 buttons)            │
│ ?????????       │ ?????????        > P1 earned leather patch             │
│ ?????????       │ ?????????        > P2 bought [0]                       │
│ ?????????       │ ?????????        >                                     │
│ ?????????       │ ?????????                                              │
│ ?????????       │ ?????????                                              │
│ ?????????       │ ?????????                                              │
│ ?????????       │ ?????????                                              │
├─────────────────┴────────────────────────────────────────────────────────┤
│ ndjson log (5 lines) ────────────────────────────── [m]↕ [f]⤢ [h]½↕ [,.] │
│ {"type":"move","player":0,"patch":1}                                     │
│ {"type":"move","player":1,"action":"advance","buttons_gained":4}         │
└──────────────────────────────────────────────────────────────────────────┘
```

### Wide layout — 160 columns

At ≥160 columns a two-column layout is used: the left column holds the patch circle, adaptive detail, stats, time track, and controls; the right column holds the two quilts side by side and the event log. The NDJSON pane spans the full width at the bottom and gains proportionally more lines.

```txt
┌─ PATCHWORK ── seed 42 / setup 0 ─────────────────────────────┬───────────────────────────────────────────────────────────────────────── ▶ P1 ─┐
│ Circle: ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567                    │  P1 quilt (9×9)     P2 quilt (9×9)     Event log                               │
│                  ^                                           │  ?????????          ?????????          > P1 bought [1] (+1 income)             │
│  [0] A  cost 3  time 2  income 1                             │  ?????????          ?????????          > P2 advanced (+4 buttons)              │
│  [1] B  cost 5  time 3  income 2                             │  ?????????          ?????????          > P1 earned leather patch               │
│  [2] C  cost 2  time 1  income 0                             │  ?????????          ?????????          > P2 bought [0]                         │
│  [3] D  cost 1  time 1  income 0                             │  ?????????          ?????????          >                                       │
│  [4] E  cost 4  time 2  income 1                             │  ?????????          ?????????                                                  │
│                                                              │  ?????????          ?????????                                                  │
│  P1  btn 14  inc 3  pos 12  fr 74                            │  ?????????          ?????????                                                  │
│  P2  btn 11  inc 2  pos 20  fr 81                            │  ?????????          ?????????                                                  │
│  time: ──P1─────────────────P2──────── 0..53                 │                                                                                │
│  [0-x]buy  [a]adv  [q]quit                                   │                                                                                │
│  [z/u]undo  [Z/r]redo  [</>]log  [w]wrap                     │                                                                                │
│  [m]ndjson↕  [f]ndjson⤢  [h]ndjson½↕  [,]decrL  [.]incrL     │                                                                                │
├──────────────────────────────────────────────────────────────┴────────────────────────────────────────────────────────────────────────────────┤
│ ndjson log (12 lines) ────────────────────────────────────────────────────────────────────────────────────────────────── [m]↕ [f]⤢ [h]½↕ [,.] │
│ {"type":"move","player":0,"patch":1,"cost":3,"time_cost":2,"income_gain":1}                                                                   │
│ {"type":"move","player":1,"action":"advance","buttons_gained":4,"new_position":20}                                                            │
│ {"type":"income","player":0,"amount":3,"new_total":14}                                                                                        │
│ {"type":"leather_patch","player":0,"position":26}                                                                                             │
└───────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
```

Column budget at 80: quilt section ~30 cols, event log ~46 cols, ndjson 5 lines (default). At 120 cols the log pane gains ~40 columns. At ≥160 cols the two-column layout activates.

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

### 8. Responsive layout: 80-column baseline, two-column at ≥160

**Decision:** The rendering function queries the current terminal width on each frame via `ioctl TIOCGWINSZ`. In the narrow layout (80–159 cols): the event-log pane gets all columns beyond the fixed left section (quilt + stats + separators ≈ 32 cols); the time-track bar scales to fill the available width. In the wide layout (≥160 cols): a two-column arrangement is used — the left column (fixed ~65 cols) holds the patch circle, detail lines, player stats, time track, and all keyboard shortcuts; the right column holds both 9×9 quilts side by side and the event log. The NDJSON pane always spans the full terminal width at the bottom. A minimum of 80 columns is enforced at startup.

**Rationale:** The wide layout leverages extra screen real estate by showing the circle and the quilts simultaneously without hiding any information. The two-column split is chosen at 160 rather than 120 because the quilts + log together need ~95 cols comfortably after the left column.

**Alternative considered:** A single threshold-less elastic layout — rejected because the jump from "patch circle above quilts" to "patch circle beside quilts" is a structural change that cannot be expressed as a continuous resize.

### 9. NDJSON log pane with resize controls

**Decision:** The NDJSON log section occupies a configurable number of lines at the bottom of the frame. Four keyboard shortcuts control its size:

- `m` — toggle minimize: collapses the pane to 0 lines (header bar only) and restores to the last non-zero height on the next press
- `f` — maximize: expands the pane to fill all remaining terminal rows below the main frame
- `h` — semi-maximize: sets the pane height to `floor((max_ndjson_lines) / 2)`, i.e., half the maximum
- `[` / `]` — decrement / increment pane height by 1 line (clamped to 0 … max)

The default height is 5 lines. The event log and horizontal scroll behaviour are unchanged and apply to the separate in-frame event log pane.

**Rationale:** A fixed-height NDJSON pane wastes space when debugging is not needed and hides information when inspecting move sequences. The three preset shortcuts (min/max/half) cover the common workflows; fine-tuning with `[`/`]` handles edge cases. Mirroring the shortcuts in both the main frame hint line and the NDJSON header bar keeps them discoverable.

**Alternative considered:** A separate `--ndjson-lines N` flag at launch — rejected because runtime adjustment is more ergonomic than restarting the binary.

### 3. Single-file `tui_main.cpp` entry point, modular headers

**Decision:** Four translation units under `src/tui/`: `display.cpp`, `input.cpp`, `history.cpp`, `launch.cpp`, plus a `tui_main.cpp` entry point. Each has a corresponding `.hpp`.

**Rationale:** Keeps each concern independently testable. `history.cpp` and `display.cpp` have no terminal I/O and are straightforward to unit-test with Catch2.

### 4. Terminal raw mode scoped with RAII

**Decision:** A `RawMode` RAII guard in `input.hpp` saves/restores `termios` settings on construction/destruction.

**Rationale:** Ensures the terminal is always restored to cooked mode on exit, exception, or `SIGTERM` (via `atexit`).

### 5. Opponent is always the random agent (for this phase)

**Decision:** The launch screen lets the user pick a game setup index and seed; the opponent is always `RandomAgent`. Future phases add agent selection.

**Rationale:** No agent choice is needed until Random Sampling Agents phase. Keeping scope narrow avoids prematurely abstracting the agent interface.

### 10. Color scheme — ANSI 16-color, graceful degradation

**Decision:** The TUI uses the ANSI 16-color palette. Color assignments per concept (TBD, defined with constants), other details:

| Element                    | Color                                                                      |
|----------------------------|----------------------------------------------------------------------------|
| P1 stats / marker          | Bright cyan (`\033[96m`)                                                   |
| P2 stats / marker          | Bright yellow (`\033[93m`)                                                 |
| Affordable patch detail    | Bright green (`\033[92m`)                                                  |
| Unaffordable patch detail  | Dim white (`\033[2m`)                                                      |
| Active-player row / header | Bold (`\033[1m`)                                                           |
| Event-log prompt `>`       | Green (`\033[32m`)                                                         |
| NDJSON log text            | Dim for control characters (`\033[2m`), contents using colors for concepts |
| Error / illegal-move flash | Bold red (`\033[1;31m`)                                                    |
| Box-drawing frame          | Default foreground                                                         |

Color is suppressed when: `TERM=dumb`, `NO_COLOR` environment variable is set, or `--no-color` flag is passed.

**Rationale:** 16-color ANSI is universally supported and maps cleanly onto the game's two-player symmetry. Affordable-patch green / unaffordable dim encoding is the highest-value visual affordance for decision-making. Full 256-color or truecolor is unnecessary and would complicate the CI environment.

**Alternative considered:** No color (plain text only) — rejected because color meaningfully improves readability of the patch affordability column at a glance.

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
- Wide-layout threshold: exactly 160 columns, or configurable? → Fixed at 160 for this phase; revisit if user feedback requests a lower threshold.
