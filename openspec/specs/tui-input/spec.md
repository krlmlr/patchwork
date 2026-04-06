### Requirement: Input loop reads single keypress commands

The input module SHALL provide a blocking `read_command` function that returns a typed `Command` variant without requiring the user to press Enter. Valid commands are: `BuyPatch{int index}` (**keys `1`, `2`, `3` map to `BuyPatch{0}`, `BuyPatch{1}`, `BuyPatch{2}` respectively** — 1-indexed keys for the first three buyable patches), `Advance` (keys `a` or Space), `Undo` (keys `z` or `u`), `Redo` (keys `Z` or `r`), `ScrollLogLeft` (key `<`), `ScrollLogRight` (key `>`), `ToggleLogWrap` (key `w`), `NdjsonToggleMinimize` (key `m`), `NdjsonMaximize` (key `f`), `NdjsonSemiMaximize` (key `h`), `NdjsonDecrLines` (key `,`), `NdjsonIncrLines` (key `.`), and `Quit` (key `q` or `Q`). Any other key SHALL be silently ignored and `read_command` SHALL continue waiting.

#### Scenario: Digit key produces BuyPatch command

- **WHEN** the user presses key `2`
- **THEN** `read_command` returns `BuyPatch{1}` (second buyable patch, 0-indexed internally)

#### Scenario: 'a' key produces Advance command

- **WHEN** the user presses key `a`
- **THEN** `read_command` returns `Advance{}`

#### Scenario: Space key also produces Advance command

- **WHEN** the user presses the Space key
- **THEN** `read_command` returns `Advance{}`

#### Scenario: 'z' key produces Undo command

- **WHEN** the user presses key `z`
- **THEN** `read_command` returns `Undo{}`

#### Scenario: 'Z' key produces Redo command

- **WHEN** the user presses key `Z`
- **THEN** `read_command` returns `Redo{}`

#### Scenario: 'q' key produces Quit command

- **WHEN** the user presses key `q`
- **THEN** `read_command` returns `Quit{}`

#### Scenario: Unrecognised keys are ignored

- **WHEN** the user presses key `x`
- **THEN** `read_command` does not return and continues waiting for a valid key

#### Scenario: '<' key produces ScrollLogLeft command

- **WHEN** the user presses key `<`
- **THEN** `read_command` returns `ScrollLogLeft{}`

#### Scenario: '>' key produces ScrollLogRight command

- **WHEN** the user presses key `>`
- **THEN** `read_command` returns `ScrollLogRight{}`

#### Scenario: 'w' key produces ToggleLogWrap command

- **WHEN** the user presses key `w`
- **THEN** `read_command` returns `ToggleLogWrap{}`

#### Scenario: 'm' key produces NdjsonToggleMinimize command

- **WHEN** the user presses key `m`
- **THEN** `read_command` returns `NdjsonToggleMinimize{}`

#### Scenario: 'f' key produces NdjsonMaximize command

- **WHEN** the user presses key `f`
- **THEN** `read_command` returns `NdjsonMaximize{}`

#### Scenario: 'h' key produces NdjsonSemiMaximize command

- **WHEN** the user presses key `h`
- **THEN** `read_command` returns `NdjsonSemiMaximize{}`

#### Scenario: ',' key produces NdjsonDecrLines command

- **WHEN** the user presses key `,`
- **THEN** `read_command` returns `NdjsonDecrLines{}`

#### Scenario: '.' key produces NdjsonIncrLines command

- **WHEN** the user presses key `.`
- **THEN** `read_command` returns `NdjsonIncrLines{}`

### Requirement: RawMode guard enables and restores terminal raw mode

The input module SHALL provide a `RawMode` RAII class. Its constructor SHALL switch the terminal to raw mode (no echo, no line buffering). Its destructor SHALL restore the original `termios` settings unconditionally. The guard SHALL also register an `atexit` handler to restore settings if the process exits abnormally.

#### Scenario: Terminal is in raw mode while guard is alive

- **WHEN** a `RawMode` object is constructed
- **THEN** subsequent reads from `stdin` return each character immediately without waiting for Enter

#### Scenario: Terminal is restored when guard is destroyed

- **WHEN** a `RawMode` object goes out of scope
- **THEN** the terminal's `termios` settings match the saved settings captured at construction time

### Requirement: Command is only dispatched when it is legal

The input loop SHALL check whether the resolved `Move` is present in the legal moves list returned by `generate_moves` before applying it. Illegal commands (e.g., `BuyPatch{3}` when fewer than 4 patches are in the circle, or any move when the game is terminal) SHALL be silently ignored; the display SHALL be refreshed with no state change.

#### Scenario: Illegal BuyPatch index is ignored

- **WHEN** only 2 patches are visible in the circle and the user presses `3`
- **THEN** the game state is unchanged and no error is shown

#### Scenario: Any move after game end is ignored

- **WHEN** the game is in a terminal state
- **THEN** any key press except `z` (undo) or `q` (quit) is silently ignored
