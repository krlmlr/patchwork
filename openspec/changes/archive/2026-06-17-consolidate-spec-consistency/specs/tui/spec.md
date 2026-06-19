## RENAMED Requirements

- FROM: `### Requirement: History stack stores `(GameState, RngState)` pairs`
- TO: `### Requirement: History stack stores `(SimplifiedGameState, RngState, log-snapshot)` entries`

## MODIFIED Requirements

### Requirement: Command is only dispatched when it is legal

The input loop SHALL check whether the resolved `Move` is present in the legal moves list returned by `legal_moves` before applying it. Illegal commands (e.g., `BuyPatch{3}` when fewer than 4 patches are in the circle, or any move when the game is terminal) SHALL be silently ignored; the display SHALL be refreshed with no state change.

#### Scenario: Illegal BuyPatch index is ignored

- **WHEN** only 2 patches are visible in the circle and the user presses `3`
- **THEN** the game state is unchanged and no error is shown

#### Scenario: Any move after game end is ignored

- **WHEN** the game is in a terminal state
- **THEN** any key press except `z` (undo) or `q` (quit) is silently ignored

### Requirement: History stack stores `(SimplifiedGameState, RngState, log-snapshot)` entries

The `History` class SHALL store a sequence of `HistoryEntry` values. Each entry SHALL contain a `SimplifiedGameState` snapshot, an `RngState` snapshot capturing the full `std::mt19937` state of the random agent at that point, and a `log_entries` snapshot (a `std::vector<std::string>` of the event-log lines visible at that point) so undo/redo can restore the displayed log. The initial entry is pushed at construction. Each call to `push` appends a new entry and advances the cursor to it. When `push` is called with a cursor that is not at the end (i.e., after one or more undos), all entries above the cursor SHALL be discarded before appending.

#### Scenario: Initial entry is stored at construction

- **WHEN** a `History` is constructed with a given `SimplifiedGameState` and initial `RngState`
- **THEN** `current_state()` returns that `SimplifiedGameState`, `current_rng()` returns that `RngState`, and `can_undo()` returns `false`

#### Scenario: Push advances the cursor

- **WHEN** `push` is called with a new `SimplifiedGameState`, `RngState`, and log snapshot
- **THEN** `current_state()` returns the new state, `current_log_entries()` returns the new log snapshot, and `can_undo()` returns `true`

#### Scenario: Push after undo discards future entries

- **WHEN** the history has entries [E0, E1, E2] with cursor at E1 (after one undo) and `push(E3)` is called
- **THEN** the history contains [E0, E1, E3], `current_state()` returns E3's state, and `can_redo()` returns `false`
