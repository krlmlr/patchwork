### Requirement: Game state is rendered as a responsive box-framed ASCII layout

The TUI SHALL render the full game state into a box-drawn frame on each display update. The frame SHALL use Unicode box-drawing characters (`┌`, `─`, `┐`, `│`, `├`, `┤`, `└`, `┘`, `┬`, `┴`, `┼`) for all borders and section dividers. **Every row in the rendered frame SHALL have the same visual display width equal to the terminal column count `W`; the right border character (or corner) SHALL appear at exactly the same horizontal position on every row.** The frame SHALL contain five sections: (1) header (seed, setup, active player), (2) patch circle with adaptive detail and keyboard-shortcut legend, (3) player stats, (4) two 9×9 quilt boards side by side with the event log to the right, (5) NDJSON log pane (resizable, spanning the full width). The frame SHALL be redrawn in full on every update. A narrow layout (80–159 cols) and a wide layout (≥160 cols) SHALL be supported.

#### Scenario: All frame rows have the same visual width

- **WHEN** `render_frame` is called on an 80-column terminal
- **THEN** every printed row (after stripping ANSI escape codes) has a visual display width of exactly 80 columns

#### Scenario: Frame renders both players' stats

- **WHEN** `render_frame` is called with a `GameState` where player 0 has 14 buttons / 3 income / 74 free spaces and player 1 has 11 buttons / 2 income / 81 free spaces
- **THEN** the output contains the strings `"14"`, `"3"`, `"74"` for player 0 and `"11"`, `"2"`, `"81"` for player 1

### Requirement: Full patch circle is displayed as a character sequence

The display SHALL always show the complete 33-character patch sequence on a single line, using each patch's single-char catalog name, in circle order. A `^` marker on the line immediately below SHALL indicate the current buy-window start position. This line SHALL be present at every render regardless of how many patches remain available.

#### Scenario: Circle line contains all 33 characters

- **WHEN** `render_frame` is called at the start of a game (all 33 patches available)
- **THEN** the circle line contains exactly 33 non-space characters corresponding to the current game setup's patch names in circle order

#### Scenario: Circle line reflects removed patches

- **WHEN** patches have been bought and `render_frame` is called
- **THEN** the circle line still contains the same 33 character positions, with bought patches replaced by a placeholder character (e.g., `.`)

#### Scenario: Marker position is correct

- **WHEN** the circle marker is at position 5
- **THEN** the `^` appears directly below position 5 in the circle line

### Requirement: Patch circle shows adaptive detail lines

The display SHALL render at least 3 detail lines below the circle marker, one per buyable patch, showing: circle index, patch name, cost, time cost, and income. **Detail lines SHALL use a fixed-width index format `[%2d]` (right-aligned within square brackets, minimum 2 digits) so that all entries are visually aligned regardless of the sequential index value.** If fewer than 3 patches remain buyable, only the available patches are listed.

**Event-log entries SHALL be visually aligned**: the patch name in `"Pn bought [c]"` entries is always a single character `c`, ensuring consistent column width across all log entries regardless of the number of moves played.

#### Scenario: At least 3 detail lines are shown

- **WHEN** at least 3 patches are buyable and the terminal is exactly 80 columns wide
- **THEN** exactly 3 detail lines are shown, prefixed `[ 1]`, `[ 2]`, `[ 3]`

#### Scenario: Detail lines have uniform column alignment

- **WHEN** more than 9 detail lines are shown (terminal is tall and NDJSON pane is minimised)
- **THEN** entry 9 is rendered `[ 9]` and entry 10 is rendered `[10]`, with both the patch name and all subsequent fields starting at the same column

### Requirement: Wide layout activates at ≥160 columns

When the terminal is at least 160 columns wide, the TUI SHALL switch to a two-column layout: the left column (80 cols) contains the patch circle, adaptive detail lines, player stats, and keyboard-shortcut legend; the right column contains the two 9×9 quilts side by side and the event log. The NDJSON log pane spans the full terminal width below both columns.

#### Scenario: Wide layout is used at 160 columns

- **WHEN** `render_frame` is called with terminal width 160
- **THEN** the patch circle section and the quilt section appear in separate columns on the same row range

#### Scenario: Narrow layout is used at 159 columns

- **WHEN** `render_frame` is called with terminal width 159
- **THEN** the patch circle section appears above the quilt section (single-column layout)

### Requirement: Color is applied to key elements

The display SHALL apply ANSI 16-color codes using named constants defined in `display.hpp`. Main frame colors: P1 stats and markers (bright cyan), P2 stats and markers (bright yellow), affordable patch detail rows (bright green), unaffordable patch detail rows (dim), active-player header bold, event-log prompt `>` (green), error/illegal-move flash (bold red). NDJSON log pane colors: structural characters `{}:,` (dim), `"player"` value `0` (bright cyan), `"player"` value `1` (bright yellow), `"move_type"` value `"buy_patch"` (bright green), `"move_type"` value `"advance"` (cyan), `"event"` values `"game_start"` and `"game_end"` (bold), `"winner"` value `0` (bright cyan), `"winner"` value `1` (bright yellow). All color SHALL be suppressed when `TERM=dumb`, `NO_COLOR` is set, or `--no-color` is passed.

#### Scenario: Affordable patches are rendered in green

- **WHEN** `render_frame` is called and patch [0] is affordable for the active player
- **THEN** the output contains ANSI green escape codes surrounding patch [0]'s detail line

#### Scenario: Color is suppressed when NO_COLOR is set

- **WHEN** `NO_COLOR` environment variable is set and `render_frame` is called
- **THEN** the output contains no ANSI escape codes

### Requirement: NDJSON log pane is resizable

The bottom NDJSON log pane height (in lines) is controlled at runtime. **The initial height is set so that the frame fills the entire terminal minus the last line** (i.e., `cfg.height - 1` total rows). Four keyboard shortcuts adjust the height: `m` (toggle minimize/restore), `f` (maximize to fill available rows), `h` (semi-maximize to `floor(max / 2)` lines), and `,` / `.` (decrement / increment by 1, clamped to 0 … max). A header bar for the NDJSON pane is always visible (1 line) even when the height is 0, showing the current height and the shortcuts. **When the NDJSON pane height decreases the freed rows are used for additional patch-circle detail lines, and vice versa** (the total frame height stays constant at `cfg.height - 1`). The NDJSON height is clamped at render time so it never overflows the terminal.

#### Scenario: Minimize hides all NDJSON lines

- **WHEN** the NDJSON pane is at its default height and `m` is pressed
- **THEN** the rendered output shows 0 NDJSON content lines (only the header bar)

#### Scenario: Restore returns to previous height

- **WHEN** the NDJSON pane is minimized and `m` is pressed again
- **THEN** the rendered output shows the same number of NDJSON lines as before minimization

#### Scenario: Maximize fills remaining rows

- **WHEN** `f` is pressed and the terminal has N rows available below the main frame
- **THEN** the NDJSON pane displays N lines

#### Scenario: Semi-maximize sets half the maximum

- **WHEN** `h` is pressed and the maximum NDJSON height is 20 lines
- **THEN** the NDJSON pane displays 10 lines

#### Scenario: Increment and decrement adjust height by 1

- **WHEN** the NDJSON pane has height 5 and `.` is pressed
- **THEN** the pane has height 6

- **WHEN** the pane has height 0 and `,` is pressed
- **THEN** the pane height remains 0 (clamped at minimum)

The lower-left section of the frame SHALL permanently display two 9×9 grids labelled "P1 quilt" and "P2 quilt". In simplified mode (no quilt board tracking) every cell SHALL display the character `?`. The grid dimensions and position in the frame SHALL not change when full quilt tracking is added in a later phase; only the cell content changes.

#### Scenario: Quilt boards display `?` in simplified mode

- **WHEN** `render_frame` is called in simplified mode
- **THEN** the output contains two blocks of 9×9 `?` characters

#### Scenario: Quilt grid is exactly 9 columns × 9 rows per player

- **WHEN** `render_frame` output is parsed
- **THEN** each quilt block spans exactly 9 columns and 9 rows of content characters

### Requirement: Terminal too small triggers error and exits

`init_display` SHALL query the terminal size via `ioctl TIOCGWINSZ`. If the terminal reports fewer than 80 columns or fewer than 24 rows, a message SHALL be printed to `stderr` naming the minimum dimensions and the process SHALL exit with a non-zero status.

#### Scenario: Terminal too small triggers error and exits

- **WHEN** `init_display` is called and the terminal reports fewer than 80 columns or fewer than 24 rows
- **THEN** a message is printed to `stderr` explaining the minimum size requirement and the process exits with a non-zero status

### Requirement: Event log maintains a scrolling buffer with horizontal scroll and wrap toggle

The display module SHALL maintain a bounded event log buffer of at most 50 entries. Each call to `append_log` adds a string to the tail; entries beyond the limit are discarded from the head. **An undo action SHALL remove the last entry from the log (pop-back) rather than appending a new entry; a redo action SHALL not modify the log.** The rendered frame SHALL display the most recent entries that fit in the log pane. The log SHALL support a horizontal scroll offset (shifted by `<` / `>` keys); adding a new entry SHALL reset the offset to 0 so the newest line is fully visible. A wrap toggle (key `w`) SHALL switch between truncated-with-scroll and word-wrapped display; in wrap mode the horizontal offset is ignored.

#### Scenario: Log buffer respects maximum size

- **WHEN** `append_log` is called 60 times with distinct strings
- **THEN** the log buffer contains exactly 50 entries and the first 10 strings are no longer present

#### Scenario: Most recent entries appear in rendered frame

- **WHEN** the log contains 10 entries and the log pane fits 6 lines
- **THEN** the rendered output contains the last 6 entries and not the first 4

#### Scenario: New entry resets horizontal scroll offset

- **WHEN** the scroll offset is non-zero and a new log entry is appended
- **THEN** the horizontal scroll offset is reset to 0

#### Scenario: Wrap mode ignores horizontal scroll offset

- **WHEN** wrap mode is active and the horizontal scroll offset is non-zero
- **THEN** rendered log lines are wrapped to the pane width and the offset has no effect on the output
