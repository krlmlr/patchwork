## Why

The `SimplifiedGameState` type established in the previous phase captures per-player economy state and shared metadata, but the engine cannot yet play a game: there is no logic to enumerate legal moves, apply them, detect the end of game, or record results. Without these, no agent can run and no data can be collected.

## What Changes

- **Legal move generation** — given a `SimplifiedGameState`, enumerate which patches (up to 3 ahead of the circle marker) each player may legally buy, plus the "advance and earn 1 button per space" move
- **Move application** — mutate or return a new `SimplifiedGameState` after a move: deduct buttons, advance time, update income, decrement free spaces, remove taken patch from availability, advance circle marker, award 1×1 leather patches at correct time thresholds, check 7×7 bonus after each patch placement
- **Terminal detection** — determine when both players have reached or passed position 53
- **Scoring** — compute final score: buttons minus 2× free spaces, plus bonus tile if claimed
- **NDJSON game logging** — structured log lines for move events and game outcomes
- **Random agent** — selects uniformly at random from legal moves
- **Reproducible play driver** — runs a full game given a random seed and starting setup, logging to file

## Capabilities

### New Capabilities

- `move-generation`: Enumerate legal moves for the active player from a `SimplifiedGameState`
- `move-application`: Apply a move to a `SimplifiedGameState`, producing the successor state (includes leather patch awards and 7×7 bonus check)
- `terminal-and-scoring`: Detect game end and compute final scores for both players
- `game-logger`: NDJSON-format event log: game start, each move, game end with outcome
- `random-agent`: Uniform random move selector; reproducible via seed
- `play-driver`: Orchestrates a full game loop (agent × agent → log file) with seed + setup as inputs

### Modified Capabilities

- `simplified-game-state`: Extend `SimplifiedGameState` with any helpers required by move generation and application (e.g., active-player query, leather-patch threshold constants). No requirement changes to existing fields.

## Impact

- New source files under `cpp/` for move types, move generation, move application, terminal/scoring logic, game logger, random agent, and play driver
- New test files under `tests/` for each new capability
- `codegen/` and `data/` unchanged; generated headers unchanged
- No changes to `PlayerState` / `GameState` (full spatial types)
- `logs/` directory used at runtime; not committed
