## Why

The engine can play games programmatically but has no human-facing interface, making it hard to explore game states, test strategies, or demonstrate the engine interactively. A TUI bridges that gap without requiring a graphical stack, and is the next roadmap phase after Simplified Rules.

## What Changes

- Add a terminal UI that renders the live game state (buttons, income, free spaces, time track with player positions, patch circle) in ASCII art
- Add a scrolling event/move log displayed alongside the board view
- Add single-keypress input handling for move commands: buy a patch by index, or advance
- Add unlimited undo/redo so players and developers can explore states freely
- Add a launch screen to select game configuration (engine, seed, setup) before starting a session
- The quilt board area is reserved in the layout but left as a stub for a future phase

## Capabilities

### New Capabilities

- `tui-display`: ASCII art rendering of game state — buttons, income, free spaces, time track with player positions, and the visible patch circle
- `tui-input`: single-keypress input loop for move selection (pick patch by index, advance) and system commands (quit, undo, redo)
- `tui-undo-redo`: history stack enabling unlimited undo and redo of moves during a session
- `tui-launch`: startup configuration screen for choosing engine variant, game setup, and random seed before entering the main game loop

### Modified Capabilities

## Impact

- New source files under `cpp/tui/` (display, input, history, launch)
- No changes to existing game engine, state types, or logging code
- New Meson build target linking `tui/` sources with the existing engine library
- New Catch2 tests for undo/redo history and display formatting helpers
- No new third-party dependencies; uses only ANSI escape codes and standard terminal I/O
