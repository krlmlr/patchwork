# tui-undo-redo Specification

## Purpose
Defines the undo/redo history stack that stores `(GameState, RngState)` pairs to support stepping backward and forward through game moves in the TUI.

## Requirements

### Requirement: History stack stores `(GameState, RngState)` pairs

The `History` class SHALL store a sequence of `HistoryEntry` values, where each entry pairs a `GameState` snapshot with an `RngState` snapshot capturing the full `std::mt19937_64` state of the random agent at that point. The initial entry is pushed at construction. Each call to `push` appends a new entry and advances the cursor to it. When `push` is called with a cursor that is not at the end (i.e., after one or more undos), all entries above the cursor SHALL be discarded before appending.

#### Scenario: Initial entry is stored at construction

- **WHEN** a `History` is constructed with a given `GameState` and initial `RngState`
- **THEN** `current_state()` returns that `GameState`, `current_rng()` returns that `RngState`, and `can_undo()` returns `false`

#### Scenario: Push advances the cursor

- **WHEN** `push` is called with a new `GameState` and `RngState`
- **THEN** `current_state()` returns the new `GameState` and `can_undo()` returns `true`

#### Scenario: Push after undo discards future entries

- **WHEN** the history has entries [E0, E1, E2] with cursor at E1 (after one undo) and `push(E3)` is called
- **THEN** the history contains [E0, E1, E3], `current_state()` returns E3's state, and `can_redo()` returns `false`

### Requirement: Undo and redo move the cursor without losing entries

`undo` SHALL decrement the cursor by one if `can_undo()` is true, otherwise it SHALL be a no-op. `redo` SHALL increment the cursor by one if `can_redo()` is true, otherwise it SHALL be a no-op. Neither operation SHALL modify stored entries.

#### Scenario: Undo moves cursor back

- **WHEN** history has [E0, E1] with cursor at E1 and `undo` is called
- **THEN** `current_state()` returns E0's state and `can_redo()` returns `true`

#### Scenario: Redo moves cursor forward

- **WHEN** history has [E0, E1] with cursor at E0 (after undo) and `redo` is called
- **THEN** `current_state()` returns E1's state and `can_undo()` returns `true`

#### Scenario: Undo at beginning is a no-op

- **WHEN** `can_undo()` returns `false` and `undo` is called
- **THEN** `current_state()` is unchanged and no exception is thrown

#### Scenario: Redo at end is a no-op

- **WHEN** `can_redo()` returns `false` and `redo` is called
- **THEN** `current_state()` is unchanged and no exception is thrown

### Requirement: Redo produces deterministic opponent moves via saved RNG state

After redo, the random agent SHALL be reseeded with the `RngState` saved in the restored entry, ensuring that the opponent's next move is identical to the move that was originally played after that position.

#### Scenario: Opponent move after redo matches original

- **WHEN** a move sequence [player move, opponent move] has been recorded, the player undoes to before the player move, and then redoes
- **THEN** the opponent's subsequent move is identical to the original opponent move that was recorded

#### Scenario: Opponent move after new branch differs from original

- **WHEN** the player undoes and then makes a different player move (creating a new branch)
- **THEN** the opponent's move after the new branch need not match any previously recorded opponent move (it depends on the RNG state at the branch point)

### Requirement: History is unit tested

All `History` behaviours SHALL have Catch2 unit tests covering construction, push, push-after-undo truncation, undo, redo, boundary no-ops, and deterministic redo via saved `RngState`.

#### Scenario: Tests exist and pass

- **WHEN** `meson test -C build` is run
- **THEN** all history tests pass with exit code 0
