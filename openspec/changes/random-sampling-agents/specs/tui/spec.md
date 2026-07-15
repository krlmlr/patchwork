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

### Requirement: History stack stores `(SimplifiedGameState, RngState, log-snapshot)` entries

The `History` class SHALL store a sequence of `HistoryEntry` values. Each entry SHALL contain a `SimplifiedGameState` snapshot, a `log_entries` snapshot (a `std::vector<std::string>` of the event-log lines visible at that point), and — **superseding the single `RngState` field of the prior requirement** — two `RngState` snapshots: `rng_p0` (player 0 / the human; preserved for future agent-vs-agent TUI support) and `rng_p1` (player 1 / the opponent agent). Each `RngState` captures the full `std::mt19937` state of the corresponding player at that point so undo/redo can restore both the displayed log and each player's RNG stream independently. The single `current_rng()` accessor is replaced by `current_rng_p0()` and `current_rng_p1()`. The initial entry is pushed at construction. Each call to `push` appends a new entry and advances the cursor to it. When `push` is called with a cursor that is not at the end (i.e., after one or more undos), all entries above the cursor SHALL be discarded before appending.

#### Scenario: Initial entry is stored at construction

- **WHEN** a `History` is constructed with a given `SimplifiedGameState`, initial `rng_p0`, and initial `rng_p1`
- **THEN** `current_state()` returns that `SimplifiedGameState`, `current_rng_p0()` returns that `rng_p0`, `current_rng_p1()` returns that `rng_p1`, and `can_undo()` returns `false`

#### Scenario: Push advances the cursor

- **WHEN** `push` is called with a new `SimplifiedGameState`, `rng_p0`, `rng_p1`, and log snapshot
- **THEN** `current_state()` returns the new state, `current_log_entries()` returns the new log snapshot, and `can_undo()` returns `true`

#### Scenario: Push after undo discards future entries

- **WHEN** the history has entries [E0, E1, E2] with cursor at E1 (after one undo) and `push(E3)` is called
- **THEN** the history contains [E0, E1, E3], `current_state()` returns E3's state, and `can_redo()` returns `false`

#### Scenario: History entry stores both RNG states

- **WHEN** `push` is called with a new `SimplifiedGameState`, `rng_p0`, and `rng_p1`
- **THEN** `current_rng_p0()` returns `rng_p0` and `current_rng_p1()` returns `rng_p1`

### Requirement: Redo produces deterministic opponent moves via saved RNG state

After redo, the opponent agent SHALL be reseeded with the `rng_p1` state saved in the restored entry (and player 0's stream from `rng_p0`), ensuring that the opponent's next move is identical to the move that was originally played after that position.

#### Scenario: Opponent move after redo matches original

- **WHEN** a move sequence [player move, opponent move] has been recorded, the player undoes to before the player move, and then redoes
- **THEN** the opponent's `rng_p1` is restored from the history entry and the subsequent opponent move is identical to the original opponent move that was recorded

#### Scenario: Opponent move after new branch differs from original

- **WHEN** the player undoes and then makes a different player move (creating a new branch)
- **THEN** the opponent's move after the new branch need not match any previously recorded opponent move (it depends on the RNG state at the branch point)

### Requirement: History is unit tested

All `History` behaviours SHALL have Catch2 unit tests covering construction, push, push-after-undo truncation, undo, redo, boundary no-ops, and — **superseding the single-`RngState` coverage of the prior requirement** — deterministic redo via the saved per-player `rng_p0` and `rng_p1` states, including that both streams are stored on `push` and restored independently on undo/redo.

#### Scenario: Tests exist and pass

- **WHEN** `meson test -C build` is run
- **THEN** all history tests pass with exit code 0

## ADDED Requirements

### Requirement: TUI header displays the opponent agent strategy name

The header section of the rendered frame SHALL display the opponent agent's strategy name so the human player can see which type of opponent they are playing against.

#### Scenario: Strategy name appears in header

- **WHEN** `render_frame` is called after a game started with strategy `income`
- **THEN** the header section contains the string `"income"` (or a clearly labelled equivalent)
