## MODIFIED Requirements

### Requirement: Launch screen collects game configuration before starting

When `patchwork-tui` starts, it SHALL display a launch screen that prompts the user to enter: (1) a game setup index (0–99, default 0), (2) a random seed for the opponent agent (any non-negative integer, default 42), (3) an opponent agent strategy chosen from the valid strategy names (`random`, `cheap`, `income`, `income-per-time`; default `random`). The human player is always player 0; the agent is player 1. The user confirms each field with Enter. Invalid input SHALL display an inline error and re-prompt the same field without clearing earlier entries.

#### Scenario: Valid input starts the game with the given configuration

- **WHEN** the user enters setup index `3`, seed `100`, and strategy `cheap` and presses Enter
- **THEN** the game session starts with `kGameSetups[3]`, player 1 is a `cheap`-biased agent seeded with `100`

#### Scenario: Out-of-range setup index is rejected

- **WHEN** the user enters setup index `200` (out of range 0–99)
- **THEN** an error message is shown and the setup index prompt is re-displayed

#### Scenario: Non-numeric seed input is rejected

- **WHEN** the user enters a non-numeric string for the seed field
- **THEN** an error message is shown and the seed prompt is re-displayed

#### Scenario: Invalid strategy name is rejected

- **WHEN** the user enters `best` as the strategy name (not a valid strategy)
- **THEN** an error message listing valid strategy names is shown and the strategy prompt is re-displayed

#### Scenario: Empty input uses default values

- **WHEN** the user presses Enter without typing a value for any field
- **THEN** setup index defaults to `0`, seed defaults to `42`, and strategy defaults to `random`

## ADDED Requirements

### Requirement: TUI header displays the opponent agent strategy name

The header section of the rendered frame SHALL display the opponent agent's strategy name so the human player can see which type of opponent they are playing against.

#### Scenario: Strategy name appears in header

- **WHEN** `render_frame` is called after a game started with strategy `income`
- **THEN** the header section contains the string `"income"` (or a clearly labelled equivalent)

### Requirement: History stores per-player RNG states for deterministic replay

The `HistoryEntry` type SHALL store two `RngState` snapshots: `rng_p0` (the human player's — unused but preserved for future agent-vs-agent TUI support) and `rng_p1` (the opponent agent's). All existing `History` requirements about undo, redo, push-after-undo truncation, and deterministic redo apply independently to each player's RNG stream.

#### Scenario: Opponent move after redo matches original with per-player RNG

- **WHEN** the history records a human move followed by an agent move, the player undoes to before the human move, and then redoes
- **THEN** the agent's `rng_p1` is restored from the history entry and the subsequent agent move is identical to the originally recorded agent move

#### Scenario: History entry stores both RNG states

- **WHEN** `push` is called with a new `GameState`, `rng_p0`, and `rng_p1`
- **THEN** `current_rng_p0()` returns `rng_p0` and `current_rng_p1()` returns `rng_p1`
