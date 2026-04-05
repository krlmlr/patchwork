## ADDED Requirements

### Requirement: Game state is rendered as a responsive ASCII frame

The TUI SHALL render the full game state into a frame on each display update. The frame SHALL contain five sections: (1) header (seed, setup, active player), (2) patch circle with adaptive detail, (3) player stats and time track, (4) two 9×9 quilt boards side by side, (5) event log pane. The frame SHALL be redrawn in full on every update by clearing the terminal and printing from the top-left corner. The layout SHALL be designed for a minimum of 80 columns; on wider terminals the log pane and time-track bar SHALL expand to fill the extra width.

#### Scenario: Frame renders both players' stats

- **WHEN** `render_frame` is called with a `GameState` where player 0 has 14 buttons / 3 income / 74 free spaces and player 1 has 11 buttons / 2 income / 81 free spaces
- **THEN** the output contains the strings `"14"`, `"3"`, `"74"` for player 0 and `"11"`, `"2"`, `"81"` for player 1

#### Scenario: Frame renders time-track positions

- **WHEN** `render_frame` is called with a `GameState` where player 0 is at time position 12 and player 1 is at position 20
- **THEN** the output contains markers for both positions on the time-track line, with the P1 marker appearing before the P2 marker

#### Scenario: Time-track bar scales with terminal width

- **WHEN** `render_frame` is called on an 80-column terminal and again on a 120-column terminal
- **THEN** the time-track bar is wider on the 120-column terminal and both bars correctly place `P1` and `P2` markers at proportional positions

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

The display SHALL render at least 3 detail lines below the circle marker, one per buyable patch, showing: circle index, patch name, cost, time cost, and income. When the terminal is wider than 80 columns the number of additional detail lines SHALL increase adaptively (approximately one extra line per 10 extra columns). If fewer than 3 patches remain buyable, only the available patches are listed.

#### Scenario: At least 3 detail lines are shown

- **WHEN** at least 3 patches are buyable and the terminal is exactly 80 columns wide
- **THEN** exactly 3 detail lines are shown, prefixed `[0]`, `[1]`, `[2]`

#### Scenario: Extra detail lines on wider terminals

- **WHEN** the terminal is 100 columns wide and at least 5 patches are buyable
- **THEN** at least 5 detail lines are shown

#### Scenario: Fewer detail lines near end of game

- **WHEN** only 2 patches remain buyable
- **THEN** exactly 2 detail lines appear

### Requirement: Two 9×9 quilt boards are reserved in the layout

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

The display module SHALL maintain a bounded event log buffer of at most 50 entries. Each call to `append_log` adds a string to the tail; entries beyond the limit are discarded from the head. The rendered frame SHALL display the most recent entries that fit in the log pane. The log SHALL support a horizontal scroll offset (shifted by `<` / `>` keys); adding a new entry SHALL reset the offset to 0 so the newest line is fully visible. A wrap toggle (Enter key) SHALL switch between truncated-with-scroll and word-wrapped display; in wrap mode the horizontal offset is ignored.

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
