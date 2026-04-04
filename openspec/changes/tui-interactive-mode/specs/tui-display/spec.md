## ADDED Requirements

### Requirement: Game state is rendered as a fixed-layout ASCII frame

The TUI SHALL render the full game state into a fixed-layout frame on each display update. The frame SHALL contain four panes: (1) player stats (buttons, income, free spaces for each player), (2) time-track positions, (3) patch circle (up to 5 visible patches with index, symbol, cost, time cost, and income per patch), and (4) a quilt-board stub marked `[reserved]`. The frame SHALL be redrawn in full on every update by clearing the terminal and printing from the top-left corner.

#### Scenario: Frame renders both players' stats

- **WHEN** `render_frame` is called with a `GameState` where player 0 has 14 buttons / 3 income / 74 free spaces and player 1 has 11 buttons / 2 income / 81 free spaces
- **THEN** the output contains the strings `"14"`, `"3"`, `"74"` in the player-0 column and `"11"`, `"2"`, `"81"` in the player-1 column

#### Scenario: Frame renders time-track positions

- **WHEN** `render_frame` is called with a `GameState` where player 0 is at time position 12 and player 1 is at position 20
- **THEN** the output contains markers for both positions on the time-track row, with `"P1"` appearing before `"P2"` in the line

#### Scenario: Patch circle shows up to 5 patches

- **WHEN** `render_frame` is called and at least 5 patches are available in the circle
- **THEN** exactly 5 patch entries are shown, each prefixed with `"[0]"` through `"[4]"`

#### Scenario: Patch circle shows fewer entries near end of game

- **WHEN** only 2 patches remain available in the circle
- **THEN** exactly 2 patch entries appear, followed by no further `"[N]"` lines

#### Scenario: Terminal too small triggers error and exits

- **WHEN** `init_display` is called and the terminal reports fewer than 80 columns or fewer than 24 rows
- **THEN** a message is printed to `stderr` explaining the minimum size requirement and the process exits with a non-zero status

### Requirement: Event log maintains a scrolling buffer

The display module SHALL maintain a bounded event log buffer of at most 50 entries. Each call to `append_log` adds a string to the tail; entries beyond the limit are discarded from the head. The rendered frame SHALL display the most recent entries that fit in the log pane.

#### Scenario: Log buffer respects maximum size

- **WHEN** `append_log` is called 60 times with distinct strings
- **THEN** the log buffer contains exactly 50 entries and the first 10 strings are no longer present

#### Scenario: Most recent entries appear in rendered frame

- **WHEN** the log contains 10 entries and the log pane fits 6 lines
- **THEN** the rendered output contains the last 6 entries and not the first 4
