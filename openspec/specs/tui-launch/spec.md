## ADDED Requirements

### Requirement: Launch screen collects game configuration before starting

When `patchwork-tui` starts, it SHALL display a launch screen that prompts the user to enter: (1) a game setup index (0–99, default 0), (2) a random seed (any non-negative integer, default 42). The opponent is always the built-in random agent for this phase. The user confirms with Enter. Invalid input SHALL display an inline error and re-prompt the same field without clearing earlier entries.

#### Scenario: Valid input starts the game with the given configuration

- **WHEN** the user enters setup index `3` and seed `100` and presses Enter
- **THEN** the game session starts with `kGameSetups[3]` and the `RandomAgent` seeded with `100`

#### Scenario: Out-of-range setup index is rejected

- **WHEN** the user enters setup index `200` (out of range 0–99)
- **THEN** an error message is shown and the setup index prompt is re-displayed

#### Scenario: Non-numeric seed input is rejected

- **WHEN** the user enters a non-numeric string for the seed field
- **THEN** an error message is shown and the seed prompt is re-displayed

#### Scenario: Empty input uses default values

- **WHEN** the user presses Enter without typing a value for setup index or seed
- **THEN** setup index defaults to `0` and seed defaults to `42`

### Requirement: Launch screen displays minimum terminal size check

The launch screen SHALL call `init_display` before rendering, inheriting the terminal-size guard: if the terminal is too small the process exits with an error before showing any prompt.

#### Scenario: Small terminal exits before launch screen

- **WHEN** the terminal has fewer than 80 columns
- **THEN** the process exits with a non-zero status and a message naming the minimum dimensions, without displaying the launch screen

### Requirement: Game session ends with a result summary

After the game reaches a terminal state (or the user presses `q`), the TUI SHALL clear the screen and print a summary showing each player's final score (buttons minus 2× free spaces, plus 7 for the bonus tile if claimed) and declare a winner.

#### Scenario: Result summary shows correct scores

- **WHEN** the game ends with player 0 having 14 buttons and 0 free spaces (no bonus) and player 1 having 10 buttons and 4 free spaces (no bonus)
- **THEN** the summary shows player 0 score `14`, player 1 score `2`, and declares player 0 the winner

#### Scenario: Bonus tile is reflected in summary

- **WHEN** player 1 claimed the 7×7 bonus tile
- **THEN** player 1's displayed score includes an additional `+7`
