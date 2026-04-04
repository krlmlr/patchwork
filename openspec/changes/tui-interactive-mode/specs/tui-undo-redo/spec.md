## ADDED Requirements

### Requirement: History stack stores full game states

The `History` class SHALL store a sequence of `GameState` snapshots. The initial state is pushed at construction. Each call to `push` appends a new `GameState` and advances the cursor to the new entry. When `push` is called with a cursor that is not at the end (i.e., after one or more undos), all states above the cursor SHALL be discarded before appending the new state.

#### Scenario: Initial state is stored at construction

- **WHEN** a `History` is constructed with a given `GameState`
- **THEN** `current()` returns that state and `can_undo()` returns `false`

#### Scenario: Push advances the cursor

- **WHEN** `push` is called with a new `GameState`
- **THEN** `current()` returns the new state and `can_undo()` returns `true`

#### Scenario: Push after undo discards future states

- **WHEN** the history has states [S0, S1, S2] with cursor at S1 (after one undo) and `push(S3)` is called
- **THEN** the history contains [S0, S1, S3], `current()` returns S3, and `can_redo()` returns `false`

### Requirement: Undo and redo move the cursor without losing state

`undo` SHALL decrement the cursor by one if `can_undo()` is true, otherwise it SHALL be a no-op. `redo` SHALL increment the cursor by one if `can_redo()` is true, otherwise it SHALL be a no-op. Neither operation SHALL modify stored states.

#### Scenario: Undo moves cursor back

- **WHEN** history has [S0, S1] with cursor at S1 and `undo` is called
- **THEN** `current()` returns S0 and `can_redo()` returns `true`

#### Scenario: Redo moves cursor forward

- **WHEN** history has [S0, S1] with cursor at S0 (after undo) and `redo` is called
- **THEN** `current()` returns S1 and `can_undo()` returns `true`

#### Scenario: Undo at beginning is a no-op

- **WHEN** `can_undo()` returns `false` and `undo` is called
- **THEN** `current()` is unchanged and no exception is thrown

#### Scenario: Redo at end is a no-op

- **WHEN** `can_redo()` returns `false` and `redo` is called
- **THEN** `current()` is unchanged and no exception is thrown

### Requirement: History is unit tested

All `History` behaviours SHALL have Catch2 unit tests covering construction, push, push-after-undo truncation, undo, redo, and boundary no-ops.

#### Scenario: Tests exist and pass

- **WHEN** `meson test -C build` is run
- **THEN** all history tests pass with exit code 0
